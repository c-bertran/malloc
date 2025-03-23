#include "malloc.h"
#include "malloc_internal.h"
#include <stddef.h>

// Get page size using allowed function
size_t get_page_size(void) { return sysconf(_SC_PAGESIZE); }

// Find zone containing pointer without using forbidden functions
t_zone *find_zone_for_ptr(void *ptr) {
	t_zone *current = g_zones;

	while (current) {
		// Check if ptr falls within this zone
		if (ptr >= current->start &&
		    ptr < (void *)((char *)current->start + current->total_size)) {
			return current;
		}
		current = current->next;
	}

	return NULL;
}

// Calculate block address from user pointer
t_block *get_block_from_ptr(void *ptr) {
	return (t_block *)((char *)ptr - BLOCK_HEADER_SIZE);
}

bool is_valid_ptr(void *ptr) {
	t_zone *zone = find_zone_for_ptr(ptr);
	if (!zone)
		return false;

	t_block *block = get_block_from_ptr(ptr);

	// Verify block is within zone bounds
	if ((void *)block < zone->start ||
	    (void *)block >= (void *)((char *)zone->start + zone->total_size))
		return false;

	// Verify magic number
	if (block->magic != MAGIC_NUMBER)
		return false;

	// Verify not already freed
	if (block->is_free)
		return false;

	return true;
}

void log_operation(const char *operation, void *ptr, size_t size) {
#if DEBUG_MALLOC
	// Buffer for building log message
	char buffer[256];
	int pos = 0;

	// Operation type
	while (*operation)
		buffer[pos++] = *operation++;

	// Add pointer
	buffer[pos++] = '(';
	buffer[pos++] = '0';
	buffer[pos++] = 'x';

	// Convert pointer to hex
	char hex_chars[] = "0123456789ABCDEF";
	unsigned long addr = (unsigned long)ptr;
	char addr_buffer[20];
	int addr_pos = 0;

	// Convert address to hex string
	do {
		addr_buffer[addr_pos++] = hex_chars[addr & 0xF];
		addr >>= 4;
	} while (addr);

	// Copy hex string in reverse (to get correct order)
	while (addr_pos > 0)
		buffer[pos++] = addr_buffer[--addr_pos];

	// Add size if relevant
	if (size > 0) {
		buffer[pos++] = ',';
		buffer[pos++] = ' ';

		// Convert size to decimal
		char size_buffer[20];
		int size_pos = 0;
		size_t size_copy = size;

		do {
			size_buffer[size_pos++] = '0' + (size_copy % 10);
			size_copy /= 10;
		} while (size_copy);

		while (size_pos > 0)
			buffer[pos++] = size_buffer[--size_pos];

		buffer[pos++] = ' ';
		buffer[pos++] = 'b';
		buffer[pos++] = 'y';
		buffer[pos++] = 't';
		buffer[pos++] = 'e';
		buffer[pos++] = 's';
	}

	buffer[pos++] = ')';
	buffer[pos++] = '\n';

	// Write the log message
	write(2, buffer, pos); // Write to stderr
#endif
}
