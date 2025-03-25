#include "malloc.h"
#include "malloc_internal.h"

// Init global variables
t_zone *g_zones = NULL;
pthread_mutex_t g_malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

void *malloc(size_t size) {
	void *result = NULL;

	init_malloc_system();
	pthread_mutex_lock(&g_malloc_mutex);
	logger("malloc", NULL, size);
	if (size == 0)
		size = 1;
	size_t needed_size = CALC_NEEDED_SIZE(size);

	t_zone *zone = find_zone_for_size(needed_size);
	if (!zone) {
		pthread_mutex_unlock(&g_malloc_mutex);
		return NULL;
	}

	t_block *block = find_free_block(zone, needed_size);
	if (block) {
		block = split_block(block, needed_size);
		block->is_free = false;
	} else {
		pthread_mutex_unlock(&g_malloc_mutex);
		return NULL;
	}
	zone->used_blocks++;
	zone->free_space -= needed_size;

	result = (void *)((char *)block + BLOCK_METADATA_SIZE);
	logger("malloc", result, size);
	pthread_mutex_unlock(&g_malloc_mutex);
	return result;
}
