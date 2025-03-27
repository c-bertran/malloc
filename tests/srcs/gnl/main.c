#include "gnl.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int ft_printf(char *string, ...);

void test_file(char *base, const char *filename) {
	char path[1000];

	strcpy(path, base);
	strcat(path, filename);
	ft_printf("Testing file: %s\n", path);
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		ft_printf("Failed to open file: %s\n", filename);
		return;
	}

	char *line;
	int line_count = 0;

	while ((line = get_next_line(fd)) != NULL) {
		line_count++;
		free(line);
	}

	close(fd);
	ft_printf("Read %d lines from %s\n", line_count, filename);
}

int main(int argc, char **argv) {
	(void)argc;

	ft_printf("Command line arguments:\n");
	for (int i = 0; i < argc; i++)
		ft_printf("argv[%d]: %s\n", i, argv[i]);

	// Test with different buffer sizes
	ft_printf("===== Running GNL tests with custom malloc =====\n");

	const char *test_files[] = {"files/41_no_nl",
	                            "files/41_with_nl",
	                            "files/42_no_nl",
	                            "files/42_with_nl",
	                            "files/43_no_nl",
	                            "files/43_with_nl",
	                            "files/alternate_line_nl_no_nl",
	                            "files/alternate_line_nl_with_nl",
	                            "files/big_line_no_nl",
	                            "files/big_line_with_nl",
	                            "files/empty",
	                            "files/multiple_line_no_nl",
	                            "files/multiple_line_with_nl",
	                            "files/multiple_nlx5",
	                            "files/nl"};

	int num_files = sizeof(test_files) / sizeof(test_files[0]);

	for (int i = 0; i < num_files; i++)
		test_file(argv[1], test_files[i]);

	ft_printf("===== GNL tests completed =====\n");
	return 0;
}
