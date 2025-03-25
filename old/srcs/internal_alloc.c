#include "malloc.h"
#include "malloc_internal.h"

/**
 * @brief Find a suitable free block in a zone
 * @param zone Zone to search
 * @param size Size needed
 * @return Pointer to block header if found, NULL otherwise
 */
t_block *find_free_block_in_zone(t_zone *zone, size_t size) {
	if (!zone)
		return NULL;

	t_block *current = zone->blocks;
	uintptr_t zone_start = (uintptr_t)zone->start;
	uintptr_t zone_end = zone_start + zone->total_size;

	if (!current || !current->next)
		return NULL;
	while (current) {
		// SAFETY CHECK: Verify block is within zone bounds
		uintptr_t current_addr = (uintptr_t)current;
		if (current_addr < zone_start || current_addr >= zone_end) {
			// Corrupted linked list detected
			return NULL;
		}

		// Validate magic number before using any other fields
		if (current->magic != MAGIC_NUMBER) {
			// Corrupted block detected
			return NULL;
		}

		if (current->is_free && current->size >= size) {
			return current;
		}

		// Protect against circular references
		t_block *next = current->next;
		if (next == current) {
			return NULL; // Self-referencing pointer
		}

		current = next;
	}

	return NULL;
}

/**
 * @brief Split block if it's significantly larger than needed
 * @param block Block to split
 * @param size Size needed
 * @return true if split was performed, false otherwise
 */
bool split_block(t_block *block, size_t size) {
	size_t min_split_size = sizeof(t_block) + 16;

	// DEFENSIVE: Validate block size before calculations
	if (block->size <= size || block->size > 1024 * 1024 * 1024) {
		// Invalid size, don't attempt to split
		return false;
	}

	if (block->size >= size + min_split_size) {
		uintptr_t new_addr = (uintptr_t)block + sizeof(t_block) + size;
		new_addr = (new_addr + 15) & ~15;
		if (new_addr + sizeof(t_block) >
		    (uintptr_t)block + block->size + sizeof(t_block))
			return false;
		t_block *new_block = (t_block *)new_addr;

		// Initialize the new block's fields BEFORE updating pointers
		new_block->size = block->size - size - sizeof(t_block);
		new_block->is_free = true;
		new_block->magic = MAGIC_NUMBER;
		new_block->prev = block;
		new_block->next = block->next;

		// Now update the linked list
		if (block->next) {
			block->next->prev = new_block;
		}
		block->next = new_block;

		// Update original block size
		block->size = size;

		return true;
	}

	return false;
}

t_block *merge_blocks(t_block *block) {
	// SAFETY: Validate input block
	if (!block || block->magic != MAGIC_NUMBER)
		return block;

	t_block *current = block;

	// SAFETY: Validate backward merging one step at a time
	while (current->prev) {
		// Save prev pointer locally for validation
		t_block *prev_block = current->prev;

		// Basic pointer validity check
		if ((uintptr_t)prev_block % sizeof(void *) != 0) {
			// Misaligned pointer - not a valid memory address
			current->prev = NULL;
			break;
		}

		// CRITICAL: Validate memory range before accessing ANY fields
		if ((uintptr_t)prev_block < (uintptr_t)g_zones ||
		    (uintptr_t)prev_block >
		      (uintptr_t)g_zones + 100UL * 1024 * 1024 * 1024) {
			// Pointer outside reasonable memory range
			current->prev = NULL;
			break;
		}

		// Now safer to check magic number
		if (prev_block->magic != MAGIC_NUMBER) {
			current->prev = NULL;
			break;
		}

		if (!prev_block->is_free)
			break;

		current = prev_block;
	}

	// SAFETY: Forward merging with extra validity checks
	while (current->next) {
		// Save next pointer locally
		t_block *next_block = current->next;

		// CRITICAL: Basic pointer validity checks before dereferencing
		if ((uintptr_t)next_block % sizeof(void *) != 0) {
			// Misaligned pointer is invalid
			current->next = NULL;
			break;
		}

		// CRITICAL: Range check before accessing ANY fields
		if ((uintptr_t)next_block < (uintptr_t)g_zones ||
		    (uintptr_t)next_block >
		      (uintptr_t)g_zones + 100UL * 1024 * 1024 * 1024) {
			// Pointer outside reasonable memory range
			current->next = NULL;
			break;
		}

		// Now safer to check magic and fields
		if (next_block->magic != MAGIC_NUMBER) {
			current->next = NULL;
			break;
		}

		if (!next_block->is_free)
			break;

		// Continue with merging as before...
		t_block *next_next = next_block->next;

		current->size += sizeof(t_block) + next_block->size;
		current->next = next_next;

		if (next_next) {
			// Same validations for next_next...
			if ((uintptr_t)next_next % sizeof(void *) == 0 &&
			    (uintptr_t)next_next > (uintptr_t)g_zones &&
			    (uintptr_t)next_next <
			      (uintptr_t)g_zones + 100UL * 1024 * 1024 * 1024 &&
			    next_next->magic == MAGIC_NUMBER) {

				next_next->prev = current;
			} else {
				current->next = NULL;
				break;
			}
		}
	}

	return current;
}

/**
 * @brief Get the zone containing a given pointer
 * @param ptr Pointer to check
 * @return Zone containing the pointer, or NULL if invalid
 */
void *get_aligned_user_ptr(t_block *block) { return &block->padding; }

/**
 * @brief Core allocation function
 * @param size Size requested by user
 * @return Pointer to allocated memory or NULL on failure
 */
void *internal_allocation_logic(size_t size) {
	if (size > 10UL * 1024 * 1024 * 1024) { // 10GB hard limit
		log_operation("internal_alloc_max_size", NULL, size);
		return NULL;
	}
	if (size == 0) {
		log_operation("internal_alloc_zero_size", NULL, 0);
		return NULL;
	}

	// Align size to ensure proper memory alignment
	size_t aligned_size = align_size(size);

	// Determine zone type based on size
	zone_type_t zone_type = get_zone_type(aligned_size);

	// First, try to find a free block in existing zones
	t_zone *current_zone = g_zones;
	t_block *free_block = NULL;

	while (current_zone) {
		if (current_zone->type == zone_type) {
			free_block = find_free_block_in_zone(current_zone, aligned_size);
			if (free_block) {
				break;
			}
		}
		current_zone = current_zone->next;
	}

	// If no suitable free block found, create a new zone
	if (!free_block) {
		current_zone = create_zone(zone_type, aligned_size);
		if (!current_zone) {
			log_operation("create_zone", (void *)current_zone, aligned_size);
			return NULL;
		}

		// Add new zone to front of zones list
		current_zone->next = g_zones;
		g_zones = current_zone;

		// Get the first block in the new zone
		free_block = current_zone->blocks;
	}

	// Mark block as used
	free_block->is_free = false;

	// Split block if it's much larger than needed
	split_block(free_block, aligned_size);

	// Update zone metadata
	current_zone->free_space -= free_block->size;
	current_zone->used_blocks++;

	// Return pointer to user data area (after header)
	return get_aligned_user_ptr(free_block);
}

/**
 * @brief Align size to system word boundary
 * @param size Size to align
 * @return Aligned size
 */
size_t align_size(size_t size) {
	// Align to 16 bytes (common alignment for most architectures)
	size_t alignment = 16;
	return (size + alignment - 1) & ~(alignment - 1);
}

/**
 * @brief Get zone type for a given allocation size
 * @param size Size to allocate
 * @return Zone type (TINY, SMALL, or LARGE)
 */
zone_type_t get_zone_type(size_t size) {
	if (size <= TINY_MAX_SIZE) {
		return ZONE_TINY;
	} else if (size <= SMALL_MAX_SIZE) {
		return ZONE_SMALL;
	} else {
		return ZONE_LARGE;
	}
}
