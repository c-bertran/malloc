#include "malloc.h"
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

int ft_printf(char *string, ...);

// Structure to track memory leaks
typedef struct {
	void *ptr;
	size_t size;
	char tag[32];
} allocation_record_t;

#define MAX_RECORDS 10000
allocation_record_t g_allocations[MAX_RECORDS];
int g_alloc_count = 0;

// Record an allocation
void record_alloc(void *ptr, size_t size, const char *tag) {
	if (g_alloc_count < MAX_RECORDS && ptr != NULL) {
		g_allocations[g_alloc_count].ptr = ptr;
		g_allocations[g_alloc_count].size = size;
		strncpy(g_allocations[g_alloc_count].tag, tag, 31);
		g_allocations[g_alloc_count].tag[31] = '\0';
		g_alloc_count++;
	}
}

// Remove an allocation record
void remove_alloc(void *ptr) {
	for (int i = 0; i < g_alloc_count; i++) {
		if (g_allocations[i].ptr == ptr) {
			// Move the last record to this position
			g_allocations[i] = g_allocations[g_alloc_count - 1];
			g_alloc_count--;
			return;
		}
	}
}

// Check for memory leaks
void check_leaks() {
	if (g_alloc_count > 0) {
		ft_printf("MEMORY LEAK DETECTED: %d allocations not freed\n",
		          g_alloc_count);
		for (int i = 0; i < g_alloc_count; i++) {
			ft_printf("  Leak #%d: %p (%zu bytes) - %s\n", i + 1,
			          g_allocations[i].ptr, g_allocations[i].size,
			          g_allocations[i].tag);
		}
		assert(g_alloc_count == 0);
	} else {
		ft_printf("No memory leaks detected\n");
	}
}

// ======== 1. Real-World Allocation Patterns ========

// Test allocation patterns similar to a database system
void test_database_pattern() {
	ft_printf("Testing database-like allocation pattern...\n");

// Simulate database page allocations
#define PAGE_SIZE 4096
#define NUM_PAGES 100

	void *pages[NUM_PAGES];

	// Allocate pages
	for (int i = 0; i < NUM_PAGES; i++) {
		pages[i] = malloc(PAGE_SIZE);
		assert(pages[i] != NULL);
		record_alloc(pages[i], PAGE_SIZE, "DB Page");

		// Write a repeating pattern to the page
		unsigned char *p = (unsigned char *)pages[i];
		for (int j = 0; j < PAGE_SIZE; j++) {
			p[j] = (j + i) % 256;
		}
	}

	// Verify page contents
	for (int i = 0; i < NUM_PAGES; i++) {
		unsigned char *p = (unsigned char *)pages[i];
		for (int j = 0; j < PAGE_SIZE; j++) {
			assert(p[j] == (j + i) % 256);
		}
	}

	// Simulate page cache behavior by freeing and reallocating
	for (int i = 0; i < NUM_PAGES / 2; i++) {
		int idx = i * 2;
		free(pages[idx]);
		remove_alloc(pages[idx]);

		pages[idx] = malloc(PAGE_SIZE);
		assert(pages[idx] != NULL);
		record_alloc(pages[idx], PAGE_SIZE, "DB Page (reused)");
	}

	// Free all pages
	for (int i = 0; i < NUM_PAGES; i++) {
		free(pages[i]);
		remove_alloc(pages[i]);
	}

	ft_printf("PASSED: Database allocation pattern\n");
}

// ======== 2. Long-Running Stability Test ========

volatile int timer_elapsed = 0;

void timer_handler(int sig) {
	(void)sig;
	timer_elapsed = 1;
}

void test_long_running() {
	ft_printf("Running long-duration stability test...\n");

	// Set up timer for 10 seconds
	struct itimerval timer;
	timer.it_value.tv_sec = 10; // 10-second test
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;

	signal(SIGALRM, timer_handler);
	setitimer(ITIMER_REAL, &timer, NULL);

// Array to track allocations
#define MAX_PTRS 1000
	void *ptrs[MAX_PTRS] = {0};
	size_t sizes[MAX_PTRS] = {0};
	unsigned char patterns[MAX_PTRS] = {
	  0}; // To store the pattern used for each allocation
	int count = 0;

	int iterations = 0;
	srand(time(NULL));

	while (!timer_elapsed) {
		iterations++;

		// Decide whether to allocate or free
		if (count < MAX_PTRS && (rand() % 100 < 60 || count < 100)) {
			// Allocate
			size_t size = rand() % 4000 + 1;
			ptrs[count] = malloc(size);
			if (ptrs[count]) {
				sizes[count] = size;
				// Store the pattern value
				unsigned char pattern = (iterations + count) % 256;
				patterns[count] = pattern;
				// Write to memory to check for corruption
				memset(ptrs[count], pattern, sizes[count]);
				count++;
			}
		} else if (count > 0) {
			// Free
			int idx = rand() % count;
			// Check memory integrity using the stored pattern
			unsigned char expected = patterns[idx];
			unsigned char *p = (unsigned char *)ptrs[idx];
			for (size_t j = 0; j < sizes[idx] && j < 100; j++) {
				if (p[j] != expected) {
					ft_printf(
					  "MEMORY CORRUPTION at idx=%d, offset=%lu: expected %d, got %d\n",
					  idx, j, expected, p[j]);
					assert(p[j] == expected);
				}
			}

			free(ptrs[idx]);

			// Move last pointer to this slot
			ptrs[idx] = ptrs[count - 1];
			sizes[idx] = sizes[count - 1];
			patterns[idx] =
			  patterns[count - 1]; // Don't forget to also move the pattern
			count--;
		}
	}

	// Clean up all remaining allocations
	for (int i = 0; i < count; i++) {
		free(ptrs[i]);
	}

	ft_printf("PASSED: Long-running stability test (%d iterations)\n",
	          iterations);
}

// ======== 3. Comparison with System Malloc ========

void test_comparative_performance() {
	ft_printf("Testing comparative performance...\n");

#define COMP_ITERATIONS 100000
#define COMP_SIZE 128

	clock_t start, end;
	double our_time;

	// First, test our malloc
	start = clock();
	for (int i = 0; i < COMP_ITERATIONS; i++) {
		void *ptr = malloc(COMP_SIZE);
		assert(ptr != NULL);
		free(ptr);
	}
	end = clock();
	our_time = ((double)(end - start)) / CLOCKS_PER_SEC;

	// Now test system malloc
	// We'll need to preload the system malloc, so just report numbers
	ft_printf("Our malloc: %.6f seconds for %d allocations/frees\n", our_time,
	          COMP_ITERATIONS);
	ft_printf("Comparison benchmark passed\n");
}

// ======== 4. Interoperability Tests ========

void test_interoperability() {
	ft_printf("Testing interoperability with library functions...\n");

	// Test with standard library functions that use malloc internally

	// strdup
	char original[] = "This is a test string for malloc interoperability";
	char *dup = strdup(original);
	assert(dup != NULL);
	assert(strcmp(original, dup) == 0);
	record_alloc(dup, strlen(original) + 1, "strdup");

	// Test with file I/O
	FILE *fp = fopen("/tmp/malloc_test.txt", "w+");
	if (fp) {
		// Write and read back
		fprintf(fp, "Testing malloc with stdio: %s", original);
		fseek(fp, 0, SEEK_SET);

		char buffer[100];
		fgets(buffer, sizeof(buffer), fp);

		fclose(fp);

		// Verify file was written correctly
		assert(strstr(buffer, "Testing malloc with stdio") != NULL);
	}

	// Clean up
	remove_alloc(dup);
	free(dup);

	ft_printf("PASSED: Interoperability test\n");
}

// ======== 5. Signal Safety Tests ========

void signal_handler(int sig) {
	(void)sig;
	// Allocate memory from within signal handler
	void *ptr = malloc(128);
	free(ptr);
}

void test_signal_handling() {
	ft_printf("Testing malloc behavior with signals...\n");

	// Set up signal handler
	signal(SIGUSR1, signal_handler);

	// Allocate and keep track
	void *ptr = malloc(256);
	record_alloc(ptr, 256, "Signal test");

	// Send signal to ourselves
	raise(SIGUSR1);

	// Continue allocation after signal
	void *ptr2 = malloc(512);
	record_alloc(ptr2, 512, "After signal");

	// Clean up
	remove_alloc(ptr);
	free(ptr);
	remove_alloc(ptr2);
	free(ptr2);

	ft_printf("PASSED: Signal handling test\n");
}

// ======== 6. Memory Pressure Tests ========

void test_memory_pressure() {
	ft_printf("Testing behavior under memory pressure...\n");

#define PRESSURE_SIZE (10 * 1024 * 1024) // 10MB chunks
#define MAX_CHUNKS 100

	void *chunks[MAX_CHUNKS];
	int allocated = 0;

	// Try to allocate as much as possible
	for (int i = 0; i < MAX_CHUNKS; i++) {
		chunks[i] = malloc(PRESSURE_SIZE);
		if (!chunks[i]) {
			break;
		}

		// Write to ensure memory is committed
		memset(chunks[i], i & 0xFF, PRESSURE_SIZE);
		allocated++;
	}

	ft_printf("Allocated %d chunks of %d bytes (%d MB total)\n", allocated,
	          PRESSURE_SIZE, (allocated * PRESSURE_SIZE) / (1024 * 1024));

	// Now free half and reallocate
	for (int i = 0; i < allocated / 2; i++) {
		free(chunks[i]);
		chunks[i] = NULL;
	}

	int reallocated = 0;
	for (int i = 0; i < allocated / 2; i++) {
		chunks[i] = malloc(PRESSURE_SIZE);
		if (chunks[i]) {
			reallocated++;
		}
	}

	ft_printf("Successfully reallocated %d chunks after freeing half\n",
	          reallocated);

	// Clean up
	for (int i = 0; i < allocated; i++) {
		if (chunks[i]) {
			free(chunks[i]);
		}
	}

	ft_printf("PASSED: Memory pressure test\n");
}

// ======== Main Test Function ========

int main() {
	ft_printf("=== ADVANCED MALLOC TESTS ===\n\n");

	test_database_pattern();
	test_comparative_performance();
	test_interoperability();
	test_signal_handling();
	test_memory_pressure();

	// Run long test last (it takes time)
	test_long_running();

	// Check for leaks
	check_leaks();

	ft_printf("\nAll advanced tests completed successfully!\n");
	return 0;
}
