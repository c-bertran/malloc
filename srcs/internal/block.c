#include "malloc.h"
#include "malloc_internal.h"

t_block *find_free_block(t_zone *zone, size_t size) {
	t_block *block = zone->blocks;

	// First-fit approach
	while (block) {
		if (block->is_free && block->size >= size)
			return block;
		block = block->next;
	}

	return NULL;
}

t_block *split_block(t_block *block, size_t size) {
	// Check if block can be split
	size_t required_size = ALIGN(size);
	size_t remaining = block->size - required_size;

	// Not enough space to create a useful new block
	if (remaining < BLOCK_METADATA_SIZE + MALLOC_ALIGNMENT)
		return block;

	// Create new block after the current one
	t_block *new_block =
	  (t_block *)((char *)block + BLOCK_METADATA_SIZE + required_size);
	new_block->size = remaining - BLOCK_METADATA_SIZE;
	new_block->is_free = true;
	new_block->magic = MAGIC_NUMBER;
	new_block->offset = block->offset + BLOCK_METADATA_SIZE + required_size;

	// Link the new block
	new_block->next = block->next;
	if (new_block->next)
		new_block->next->prev = new_block;
	new_block->prev = block;
	block->next = new_block;

	// Update original block
	block->size = required_size;

	return block;
}

t_block *merge_blocks(t_block *block) {
	if (!block->is_free)
		return block;

	// Merge with next block if it's free
	if (block->next && block->next->is_free) {
		block->size += BLOCK_METADATA_SIZE + block->next->size;
		block->next = block->next->next;
		if (block->next)
			block->next->prev = block;
	}

	// Merge with previous block if it's free
	if (block->prev && block->prev->is_free) {
		block->prev->size += BLOCK_METADATA_SIZE + block->size;
		block->prev->next = block->next;
		if (block->next)
			block->next->prev = block->prev;
		block = block->prev;
	}

	return block;
}

t_block *init_block(t_zone *zone, size_t offset, size_t size) {
	t_block *block = (t_block *)((char *)zone->start + offset);

	block->size = size;
	block->is_free = false;
	block->magic = MAGIC_NUMBER;
	block->offset = offset;
	block->next = NULL;
	block->prev = NULL;

	// Update zone metadata
	zone->used_blocks++;
	zone->free_space -= (BLOCK_METADATA_SIZE + size);

	return block;
}

void *block_memcpy(void *dest, const void *src, size_t n) {
	char *d = (char *)dest;
	const char *s = (const char *)src;

	if (!dest || !src)
		return dest;
	for (size_t i = 0; i < n; i++)
		d[i] = s[i];
	return dest;
}

void *block_memset(void *s, int c, size_t n) {
	unsigned char *p = (unsigned char *)s;

	if (!s)
		return s;
	for (size_t i = 0; i < n; i++)
		p[i] = (unsigned char)c;
	return s;
}
