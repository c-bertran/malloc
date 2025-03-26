#include "malloc.h"
#include "malloc_internal.h"

float calculate_fragmentation(t_zone *zone) {
	if (!zone || !zone->blocks)
		return 0.0f;

	size_t total_free = 0;
	size_t largest_free = 0;
	size_t free_count = 0;
	size_t block_count = 0;

	t_block *block = zone->blocks;
	while (block) {
		block_count++;
		if (block->is_free) {
			free_count++;
			total_free += block->size;
			if (block->size > largest_free)
				largest_free = block->size;
		}
		block = block->next;
	}

	// No free blocks or only one - no fragmentation
	if (free_count <= 1 || total_free == 0)
		return 0.0f;

	// Calculate fragmentation ratio:
	// 1.0 = unfragmented (one large free block)
	// Higher values = more fragmented
	return (float)free_count * total_free / (largest_free * largest_free);
}

/**
 * Defragment memory by consolidating free blocks
 * Returns number of zones defragmented
 */
int defragment_memory(void) {
	int zones_defragged = 0;
	t_zone *zone = g_zones;

	while (zone) {
		t_zone *next_zone = zone->next;
		t_block *block = zone->blocks;

		t_bool coalesced = false;
		while (block) {
			t_block *next_block = block->next;
			// Merge adjacent free blocks
			if (block->is_free && next_block && next_block->is_free) {
				block->size += BLOCK_METADATA_SIZE + next_block->size;
				block->next = next_block->next;
				if (next_block->next)
					next_block->next->prev = block;
				coalesced = true;
			}
			block = block->next;
		}
		if (coalesced)
			zones_defragged++;
		zone = next_zone;
	}

	return zones_defragged;
}
