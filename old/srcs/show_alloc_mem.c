#include "malloc.h"
#include "malloc_internal.h"

void ft_putnbr(size_t n, int base, char *charset, int fd) {
	char buffer[32];
	int i = 0;

	if (n == 0) {
		write(fd, "0", 1);
	} else {
		while (n) {
			buffer[i++] = charset[n % base];
			n /= base;
		}
		while (i > 0)
			write(fd, &buffer[--i], 1);
	}
}

void ft_putstr(char *str, int fd) {
	while (*str)
		write(fd, str++, 1);
}

static void ft_putaddr(void *addr, int fd) {
	ft_putstr("0x", fd);
	ft_putnbr((size_t)addr, 16, "0123456789ABCDEF", fd);
}

static void print_mem() {
	size_t total_bytes = 0;
	t_zone *zone = g_zones;

	ft_putstr("\n===== MEMORY BLOCK SUMMARY =====\n", 1);
	while (zone) {
		if (zone->type == ZONE_TINY)
			ft_putstr("TINY : ", 1);
		else if (zone->type == ZONE_SMALL)
			ft_putstr("SMALL : ", 1);
		else
			ft_putstr("LARGE : ", 1);
		ft_putaddr(zone->start, 1);
		ft_putstr("\n", 1);

		t_block *block = zone->blocks;
		while (block) {
			if (!block->is_free) {
				void *start = &(block->padding), *end = (char *)start + block->size - 1;
				ft_putaddr(start, 1);
				ft_putstr(" - ", 1);
				ft_putaddr(end, 1);
				ft_putstr(" : ", 1);
				ft_putnbr(block->size, 10, "0123456789", 1);
				ft_putstr(" bytes\n", 1);
				total_bytes += block->size;
			}
			block = block->next;
		}
		zone = zone->next;
	}
	ft_putstr("Total : ", 1);
	ft_putnbr(total_bytes, 10, "0123456789", 1);
	ft_putstr(" bytes\n", 1);
}

static void print_hex_dump(void *addr, size_t size) {
	unsigned char *data = (unsigned char *)addr;

	for (size_t i = 0; i < size; i++) {
		// New line every 16 bytes
		if (i % 16 == 0) {
			if (i > 0)
				ft_putstr("\n", 1);
			ft_putaddr((void *)(data + i), 1);
			ft_putstr(": ", 1);
		}

		// Print byte in hex
		if (data[i] < 16)
			ft_putstr("0", 1); // Leading zero for single digit
		ft_putnbr(data[i], 16, "0123456789abcdef", 1);
		ft_putstr(" ", 1);

		// Print ASCII representation after 16 bytes
		if ((i + 1) % 16 == 0 || i == size - 1) {
			// Padding for incomplete line
			for (size_t j = 0; j < 15 - (i % 16); j++)
				ft_putstr("   ", 1);

			ft_putstr(" | ", 1);

			// Print ASCII chars (printable only)
			for (size_t j = i - (i % 16); j <= i; j++) {
				if (data[j] >= 32 && data[j] <= 126)
					write(1, &data[j], 1);
				else
					ft_putstr(".", 1);
			}
		}
	}
	ft_putstr("\n", 1);
}

void show_alloc_mem(void) {
	pthread_mutex_lock(&g_malloc_mutex);
	print_mem();
	pthread_mutex_unlock(&g_malloc_mutex);
}

void show_alloc_mem_ex(void) {
	pthread_mutex_lock(&g_malloc_mutex);
	size_t total_bytes = 0, total_zones = 0, total_used = 0, total_free = 0,
	       total_blocks = 0, total_free_blocks = 0, total_max_free_blocks = 0;
	t_zone *zone = g_zones;

	ft_putstr("\n===== MEMORY BLOCK DETAIL =====\n", 1);
	while (zone) {
		++total_zones;
		total_bytes += zone->total_size;

		t_block *block = zone->blocks;
		while (block) {
			++total_blocks;
			if (block->is_free) {
				total_free += block->size;
				++total_free_blocks;
				if (block->size > total_max_free_blocks)
					total_max_free_blocks = block->size;
			} else {
				size_t dump_size = block->size < 64 ? block->size : 64;

				total_used += block->size;
				ft_putstr("Block at ", 1);
				ft_putaddr(block, 1);
				ft_putstr(":\n", 1);
				ft_putstr("  Size: ", 1);
				ft_putnbr(block->size, 10, "0123456789", 1);
				ft_putstr(" bytes\n", 1);
				ft_putstr("  Magic: 0x", 1);
				ft_putnbr(block->magic, 16, "0123456789ABCDEF", 1);
				ft_putstr("\n", 1);
				ft_putstr("  Data preview:\n", 1);
				print_hex_dump(&(block->padding), dump_size);
				ft_putstr("\n", 1);
			}
			block = block->next;
		}
		zone = zone->next;
	}

	ft_putstr("\n===== MEMORY ALLOCATION STATISTICS =====\n", 1);
	ft_putstr("Total zones: ", 1);
	ft_putnbr(total_zones, 10, "0123456789", 1);
	ft_putstr("\nTotal memory: ", 1);
	ft_putnbr(total_bytes, 10, "0123456789", 1);
	ft_putstr(" bytes\n", 1);
	ft_putstr("Memory usage: ", 1);
	ft_putnbr(total_used, 10, "0123456789", 1);
	ft_putstr(" bytes used, ", 1);
	ft_putnbr(total_free, 10, "0123456789", 1);
	ft_putstr(" bytes free\n", 1);
	if (total_bytes > 0) {
		ft_putstr("Usage ratio: ", 1);
		ft_putnbr((total_used * 100) / total_bytes, 10, "0123456789", 1);
		ft_putstr("% used, ", 1);
		ft_putnbr((total_free * 100) / total_bytes, 10, "0123456789", 1);
		ft_putstr("% free\n", 1);
	}

	ft_putstr("\n===== MEMORY FRAGMENGATION STATISTICS =====\n", 1);
	ft_putstr("Fragmentation metrics:\n", 1);
	ft_putstr("  Total blocks: ", 1);
	ft_putnbr(total_blocks, 10, "0123456789", 1);
	ft_putstr("\n  Free blocks: ", 1);
	ft_putnbr(total_free_blocks, 10, "0123456789", 1);
	ft_putstr("\n  Fragmentation: ", 1);
	if (total_blocks > 0) {
		ft_putnbr((total_free_blocks * 100) / total_blocks, 10, "0123456789", 1);
		ft_putstr("%\n", 1);
	}
	ft_putstr("  Largest free block: ", 1);
	ft_putnbr(total_max_free_blocks, 10, "0123456789", 1);
	ft_putstr(" bytes\n", 1);

	print_mem();

	pthread_mutex_unlock(&g_malloc_mutex);
}
