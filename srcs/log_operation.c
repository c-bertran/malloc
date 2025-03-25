#include "malloc_internal.h"

void log(const char *operation, void *ptr, size_t size) {
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
#else
	(void)operation;
	(void)ptr;
	(void)size;
#endif
}
