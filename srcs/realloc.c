#include "malloc.h"
#include "malloc_internal.h"

void *realloc(void *ptr, size_t size) {
	if (ptr == NULL)
		return malloc(size);

	if (size == 0) {
		free(ptr);
		return NULL;
	}

	pthread_mutex_lock(&g_malloc_mutex);

	t_zone *zone = find_zone_containing(ptr);
	if (!zone) {
		pthread_mutex_unlock(&g_malloc_mutex);
		return NULL;
	}

	t_block *block = (t_block *)((char *)ptr - BLOCK_METADATA_SIZE);

	if (!verify_block(block)) {
		pthread_mutex_unlock(&g_malloc_mutex);
		return NULL;
	}

	size_t needed_size = CALC_NEEDED_SIZE(size);

	// Case 1: Current block is big enough
	if (block->size >= needed_size) {
		// We can split the block if it's significantly larger
		if (block->size > needed_size + BLOCK_METADATA_SIZE + MALLOC_ALIGNMENT) {
			block = split_block(block, needed_size);
		}
		pthread_mutex_unlock(&g_malloc_mutex);
		return ptr;
	}

	// Case 2: Try to merge with next block if it's free
	if (block->next && block->next->is_free &&
	    block->size + BLOCK_METADATA_SIZE + block->next->size >= needed_size) {

		// Merge with next block
		block->size += BLOCK_METADATA_SIZE + block->next->size;
		block->next = block->next->next;
		if (block->next)
			block->next->prev = block;

		// Split if needed
		if (block->size > needed_size + BLOCK_METADATA_SIZE + MALLOC_ALIGNMENT) {
			block = split_block(block, needed_size);
		}

		pthread_mutex_unlock(&g_malloc_mutex);
		return ptr;
	}

	// Case 3: Need to allocate new block
	pthread_mutex_unlock(&g_malloc_mutex);
	void *new_ptr = malloc(size);
	if (new_ptr) {
		block_memcpy(new_ptr, ptr, block->size < size ? block->size : size);
		free(ptr);
	}
	return new_ptr;
}
