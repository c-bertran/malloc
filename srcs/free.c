#include "malloc.h"
#include "malloc_internal.h"

static bool validate_zones_list(void) {
	t_zone *current = g_zones;
	int zone_count = 0;

	// Check each zone in the list
	while (current) {
		zone_count++;

		// Validate zone type
		if (current->type > ZONE_LARGE) {
			write(STDERR_FILENO, "ERROR: Invalid zone type\n", 25);
			return false;
		}

		// Validate zone size is reasonable
		if (current->total_size < sizeof(t_zone) ||
		    current->total_size > 1024 * 1024 * 1024) { // 1GB sanity check
			write(STDERR_FILENO, "ERROR: Suspicious zone size\n", 28);
			return false;
		}

		// Check blocks within zone
		t_block *block = current->blocks;
		size_t actual_free_space = 0;
		size_t actual_used_blocks = 0;

		// Count blocks and verify their integrity
		while (block) {
			// Check magic number
			if (block->magic != MAGIC_NUMBER) {
				write(STDERR_FILENO, "ERROR: Block has invalid magic\n", 31);
				return false;
			}

			// Track actual counts
			if (block->is_free)
				actual_free_space += block->size;
			else
				actual_used_blocks++;

			block = block->next;
		}

		// Verify counters match reality
		if (actual_used_blocks != current->used_blocks) {
			write(STDERR_FILENO, "ERROR: Block count mismatch\n", 28);
			return false;
		}

		current = current->next;
	}

	write(STDERR_FILENO, "Zones list valid\n", 17);
	return true;
}

void free(void *ptr) {
	if (!ptr) {
		log_operation("free", NULL, 0);
		return;
	}
	pthread_mutex_lock(&g_malloc_mutex);
	t_zone *zone = find_zone_for_ptr(ptr);
	if (!zone) {
		log_operation("free-invalid - zone", ptr, 0);
		pthread_mutex_unlock(&g_malloc_mutex);
		return;
	}
	t_block *block = get_block_from_ptr(ptr);
	if (!block || block->magic != MAGIC_NUMBER || block->is_free) {
		log_operation("free-invalid - block", ptr, 0);
		pthread_mutex_unlock(&g_malloc_mutex);
		return;
	}

	log_operation("free", ptr, 0);

	// Handle large allocations differently
	if (zone->type == ZONE_LARGE) {
		// Remove from zone list
		t_zone *current = g_zones;
		t_zone *prev = NULL;

		while (current && current != zone) {
			prev = current;
			current = current->next;
		}

		if (prev)
			prev->next = zone->next;
		else
			g_zones = zone->next;

		// Free the entire zone
		write(STDERR_FILENO, "Unmapping large zone\n", 21);
		munmap(zone, zone->total_size);
		write(STDERR_FILENO, "Zone unmapped successfully\n", 27);
		validate_zones_list();
	} else {
		// For TINY and SMALL zones
		block->is_free = true;
		zone->free_space += block->size;
		zone->used_blocks--;

		// Try to merge with adjacent blocks
		merge_blocks(block);
	}

	pthread_mutex_unlock(&g_malloc_mutex);
}
