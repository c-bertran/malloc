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
 * Defragment a specific zone by moving blocks
 * Returns true if defragmentation improved fragmentation
 */
static bool defragment_zone(t_zone *zone) {
	if (!zone || zone->type == ZONE_LARGE)
		return false;

	// Calculate initial fragmentation
	float initial_frag = calculate_fragmentation(zone);
	if (initial_frag < 1.5f) // Low fragmentation threshold
		return false;

	// Temporary buffer for storing block data during moves
	char buffer[SMALL_MAX_SIZE];

	// First block position after zone header
	size_t next_position = sizeof(t_zone);

	// First pass: move allocated blocks to beginning
	t_block *block = zone->blocks;
	t_block *new_head = NULL;
	t_block *last_block = NULL;

	while (block) {
		t_block *next = block->next;

		if (!block->is_free) {
			// Save block data
			size_t data_size = block->size;
			block_memcpy(buffer, (char *)block + BLOCK_METADATA_SIZE, data_size);

			// If block isn't already at the expected position
			if (block->offset != next_position) {
				// Create new block at the consolidated position
				t_block *new_block = (t_block *)((char *)zone + next_position);

				// Initialize new block
				new_block->size = data_size;
				new_block->is_free = false;
				new_block->magic = MAGIC_NUMBER;
				new_block->offset = next_position;
				new_block->next = NULL;
				new_block->prev = last_block;

				// Copy data to new position
				block_memcpy((char *)new_block + BLOCK_METADATA_SIZE, buffer,
				             data_size);

				// Update linked list
				if (last_block)
					last_block->next = new_block;
				else
					new_head = new_block;

				last_block = new_block;
			} else {
				// Block is already at the right position
				if (last_block)
					last_block->next = block;
				else
					new_head = block;

				block->prev = last_block;
				block->next = NULL;
				last_block = block;
			}

			next_position += BLOCK_METADATA_SIZE + ALIGN(data_size);
		}

		block = next;
	}

	// Create single large free block at the end if there's space
	if (next_position < zone->total_size) {
		t_block *free_block = (t_block *)((char *)zone + next_position);
		free_block->size = zone->total_size - next_position - BLOCK_METADATA_SIZE;
		free_block->is_free = true;
		free_block->magic = MAGIC_NUMBER;
		free_block->offset = next_position;
		free_block->next = NULL;
		free_block->prev = last_block;

		if (last_block)
			last_block->next = free_block;
		else
			new_head = free_block;
	}

	// Update zone metadata
	zone->blocks = new_head;

	// Calculate final fragmentation
	float final_frag = calculate_fragmentation(zone);

	return final_frag < initial_frag;
}

/**
 * Defragment memory by consolidating free blocks
 * Returns number of zones defragmented
 */
int defragment_memory(void) {
	pthread_mutex_lock(&g_malloc_mutex);

	int zones_defragged = 0;
	t_zone *zone = g_zones;

	while (zone) {
		if (defragment_zone(zone))
			zones_defragged++;
		zone = zone->next;
	}

	pthread_mutex_unlock(&g_malloc_mutex);
	return zones_defragged;
}
