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
#if DEBUG_MALLOC
			ft_putstr("ERROR: Invalid zone type\n", STDERR_FILENO);
#endif
			return false;
		}

		// Validate zone size is reasonable
		if (current->total_size < sizeof(t_zone) ||
		    current->total_size > 1024 * 1024 * 1024) { // 1GB sanity check
#if DEBUG_MALLOC
			ft_putstr("ERROR: Suspicious zone size\n", STDERR_FILENO);
#endif
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
#if DEBUG_MALLOC
				ft_putstr("ERROR: Block has invalid magic\n", STDERR_FILENO);
#endif
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
#if DEBUG_MALLOC
			ft_putstr("ERROR: Block count mismatch\n", STDERR_FILENO);
#endif
			return false;
		}

		current = current->next;
	}

#if DEBUG_MALLOC
	ft_putstr("Zones list valid\n", STDERR_FILENO);
#endif
	return true;
}

void free(void *ptr) {
	if (!ptr) {
		log_operation("free", NULL, 0);
		return;
	}
	pthread_mutex_lock(&g_malloc_mutex);

	// it's a large alignment allocation from mmap
	uintptr_t addr = (uintptr_t)ptr;
	if (addr % sysconf(_SC_PAGESIZE) == 0) {
		size_t size = *((size_t *)ptr - 1);
		munmap((void *)(addr - sizeof(size_t)), size);
		pthread_mutex_unlock(&g_malloc_mutex);
		return;
	}

	t_zone *zone = find_zone_for_ptr(ptr);
	if (!zone) {
#if DEBUG_MALLOC
		ft_putstr("free-invalid - zone\n", STDERR_FILENO);
#endif
		pthread_mutex_unlock(&g_malloc_mutex);
		return;
	}
	t_block *block = get_block_from_ptr(ptr);
	if (!block || block->magic != MAGIC_NUMBER || block->is_free) {
#if DEBUG_MALLOC
		ft_putstr("free-invalid - block\n", STDERR_FILENO);
#endif
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
#if DEBUG_MALLOC
		ft_putstr("Unmapping large zone\n", STDERR_FILENO);
#endif
		munmap(zone, zone->total_size);
#if DEBUG_MALLOC
		ft_putstr("Zone unmapped successfully\n", STDERR_FILENO);
#endif
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
