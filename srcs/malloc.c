#include "malloc.h"
#include "malloc_internal.h"

// Init global variables
t_zone *g_zones = NULL;
pthread_mutex_t g_malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

void *malloc(size_t size) {
	void *result = NULL;

	if (size > (SIZE_MAX - BLOCK_METADATA_SIZE - MALLOC_ALIGNMENT))
		return NULL;

	init_malloc_system();
	pthread_mutex_lock(&g_malloc_mutex);
	if (size == 0)
		size = 1;
	logger("malloc", NULL, size);

	if (size >= get_max_allocation_size()) {
		pthread_mutex_unlock(&g_malloc_mutex);
		return NULL;
	} // Too large for this system
	size_t needed_size = CALC_NEEDED_SIZE(size);
	if (needed_size < size) {
		pthread_mutex_unlock(&g_malloc_mutex);
		return NULL;
	} // Integer overflow check

	needed_size = ALIGN(needed_size);
	if (needed_size < size) {
		pthread_mutex_unlock(&g_malloc_mutex);
		return NULL;
	} // Another overflow check

	t_zone *zone = find_zone_for_size(needed_size);
	if (!zone) {
		pthread_mutex_unlock(&g_malloc_mutex);
		return NULL;
	}

	t_block *block = find_free_block(zone, needed_size);
	if (!block) {
		// defragment_memory();
		block = find_free_block(zone, needed_size);
		if (!block) {
			pthread_mutex_unlock(&g_malloc_mutex);
			return NULL;
		}
	}
	block = split_block(block, needed_size);
	block->is_free = false;
	zone->used_blocks++;
	zone->free_space -= needed_size;

	result = (void *)((char *)block + BLOCK_METADATA_SIZE);
	logger("malloc", result, size);
	pthread_mutex_unlock(&g_malloc_mutex);
	return result;
}
