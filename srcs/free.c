#include "malloc.h"
#include "malloc_internal.h"

void free(void *ptr) {
	pthread_mutex_lock(&g_malloc_mutex);
	if (!ptr) {
		logger("free", NULL, 0);
		pthread_mutex_unlock(&g_malloc_mutex);
		return;
	}
	t_zone *zone = find_zone_containing(ptr);
	if (!zone) {
		pthread_mutex_unlock(&g_malloc_mutex);
		return;
	}

	t_block *block = (t_block *)((char *)ptr - BLOCK_METADATA_SIZE);
	if (!verify_block(block)) {
		pthread_mutex_unlock(&g_malloc_mutex);
		return;
	}
	block->is_free = true;
	zone->used_blocks--;
	zone->free_space += block->size + BLOCK_METADATA_SIZE;
	block = merge_blocks(block);
	if (zone->type == ZONE_LARGE && zone->used_blocks == 0) {
		if (g_zones == zone)
			g_zones = zone->next;
		else {
			t_zone *prev = g_zones;
			while (prev && prev->next != zone)
				prev = prev->next;
			if (prev)
				prev->next = zone->next;
		}
		munmap(zone, zone->total_size);
	} else if (calculate_fragmentation(zone) > 1.5f)
		defragment_memory();
	logger("free", ptr, 0);
	pthread_mutex_unlock(&g_malloc_mutex);
}
