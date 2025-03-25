#include "malloc.h"
#include "malloc_internal.h"

bool is_valid_pointer(void *ptr) {
	if (!ptr)
		return false;

	t_zone *zone = find_zone_containing(ptr);
	if (!zone)
		return false;

	// Calculate pointer to block metadata
	t_block *block = (t_block *)((char *)ptr - BLOCK_METADATA_SIZE);

	return verify_block(block);
}

bool verify_block(t_block *block) {
	if (!block)
		return false;

	// Check magic number to verify it's a valid block
	if (block->magic != MAGIC_NUMBER)
		return false;

	// Check if block is currently allocated (not free)
	if (block->is_free)
		return false;

	return true;
}
