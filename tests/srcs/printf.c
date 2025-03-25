#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

static int putnbr(size_t n, int base, char *charset, int fd) {
	char buffer[32];
	int i = 0, size = 0;

	if (n == 0) {
		write(fd, "0", 1);
		return 1;
	}
	while (n) {
		buffer[i++] = charset[n % base];
		n /= base;
	}
	size = i;
	while (i > 0)
		write(fd, &buffer[--i], 1);
	return size;
}

static int putcharacter(char c, int fd) {
	write(fd, &c, 1);
	return 1;
}

static int putstr(char *str, int fd) {
	int ret = 0;

	while (*str) {
		++ret;
		write(fd, str++, 1);
	}

	return ret;
}

static int putaddr(void *addr, int fd) {
	putstr("0x", fd);
	return putnbr((size_t)addr, 16, "0123456789ABCDEF", fd) + 2;
}

int ft_printf(char *string, ...) {
	int counter = 0, i = 0;
	va_list args;

	va_start(args, string);
	while (string[i]) {
		if (string[i] == '%') {
			i++;
			if (string[i] == 's')
				counter += putstr(va_arg(args, char *), STDOUT_FILENO);
			if (string[i] == 'd')
				counter += putnbr(va_arg(args, int), 10, "0123456789", STDOUT_FILENO);
			if (string[i] == 'x')
				counter += putaddr(va_arg(args, void *), STDOUT_FILENO);
		} else
			counter += putcharacter(string[i], STDOUT_FILENO);
		i++;
	}
	va_end(args);
	return (counter);
}
