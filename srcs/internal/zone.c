#include "malloc.h"
#include "malloc_internal.h"

t_zone *create_zone(zone_type_t type, size_t size) {
	// Make sure size is page-aligned
	size = ALIGN(size);

	// Map memory for the zone
	void *zone_memory = mmap(NULL, size, PROT_READ | PROT_WRITE,
	                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (zone_memory == MAP_FAILED)
		return NULL;

	// Initialize zone structure at the beginning of the mapped memory
	t_zone *zone = (t_zone *)zone_memory;
	zone->start = zone_memory;
	zone->total_size = size;
	zone->type = type;
	zone->next = NULL;
	zone->free_space = size - sizeof(t_zone);
	zone->used_blocks = 0;
	zone->blocks = NULL;

	// Create initial free block
	t_block *block = (t_block *)((char *)zone_memory + sizeof(t_zone));
	block->next = NULL;
	block->prev = NULL;
	block->size = zone->free_space - BLOCK_METADATA_SIZE;
	block->magic = MAGIC_NUMBER;
	block->is_free = true;
	block->offset = sizeof(t_zone);

	zone->blocks = block;

	// Add to global zones list
	if (g_zones == NULL) {
		g_zones = zone;
	} else {
		zone->next = g_zones;
		g_zones = zone;
	}

	return zone;
}

t_zone *find_zone_for_size(size_t size) {
	// Using the GET_ZONE_TYPE macro
	zone_type_t type = GET_ZONE_TYPE(size);
	t_zone *zone = g_zones;

	// First pass: look for existing zone with enough space
	while (zone) {
		if (zone->type == type && zone->free_space >= size)
			return zone;
		zone = zone->next;
	}

	// No suitable zone found, create new one
	size_t zone_size;
	if (type == ZONE_TINY)
		zone_size = TINY_ZONE_SIZE;
	else if (type == ZONE_SMALL)
		zone_size = SMALL_ZONE_SIZE;
	else
		zone_size = ALIGN(size + sizeof(t_zone) + BLOCK_METADATA_SIZE);

	return create_zone(type, zone_size);
}

t_zone *find_zone_containing(void *ptr) {
	t_zone *zone = g_zones;

	while (zone) {
		if (ptr >= zone->start &&
		    ptr < (void *)((char *)zone->start + zone->total_size))
			return zone;
		zone = zone->next;
	}

	return NULL;
}
