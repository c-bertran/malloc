#include "malloc.h"
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

int ft_printf(char *string, ...);

// Test ridiculously large allocations
void test_extreme_large_allocations() {
	ft_printf("Testing absurdly large allocations...\n");

	// Try to allocate 3.5GB - should fail gracefully, not crash
	void *ptr = malloc(3UL * 1024 * 1024 * 1024 + 512 * 1024 * 1024);
	ft_printf("  3.5GB allocation: %s\n",
	          ptr ? "succeeded (unexpected)" : "failed as expected");
	free(ptr);

// Attempt SIZE_MAX allocation - should fail gracefully
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Walloc-size-larger-than="
	ptr = malloc(SIZE_MAX);
	ft_printf("  SIZE_MAX allocation: %s\n",
	          ptr ? "succeeded (unexpected)" : "failed as expected");
	free(ptr);
#pragma GCC diagnostic pop

	// Try SIZE_MAX/2 - likely to fail but tests large allocation handling
	ptr = malloc(SIZE_MAX / 2);
	ft_printf("  SIZE_MAX/2 allocation: %s\n",
	          ptr ? "succeeded (wow!)" : "failed as expected");
	free(ptr);

	ft_printf("PASSED: Extreme large allocations (didn't crash)\n");
}

// Test microscopic allocations
void test_tiny_allocations() {
	ft_printf("Testing microscopic allocations...\n");

	// Allocate 10,000 1-byte blocks
	void *ptrs[10000];
	for (int i = 0; i < 10000; i++) {
		ptrs[i] = malloc(1);
		if (ptrs[i]) {
			// Write to memory to ensure it's usable
			*((char *)ptrs[i]) = 42;
		} else {
			ft_printf("  Failed at allocation %d\n", i);
			break;
		}
	}

	// Free in reverse order
	for (int i = 9999; i >= 0; i--) {
		free(ptrs[i]);
	}

	ft_printf("PASSED: Tiny allocations\n");
}

// Test weird-sized allocations
void test_weird_sized_allocations() {
	ft_printf("Testing weirdly-sized allocations...\n");

	// Allocate prime number sizes
	int primes[] = {2,  3,  5,  7,  11, 13, 17, 19, 23, 29, 31, 37, 41,
	                43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97};
	void *prime_ptrs[sizeof(primes) / sizeof(int)];

	for (size_t i = 0; i < sizeof(primes) / sizeof(int); i++) {
		prime_ptrs[i] = malloc(primes[i]);
		assert(prime_ptrs[i] != NULL);

		// Write a pattern to verify memory
		for (int j = 0; j < primes[i]; j++) {
			((char *)prime_ptrs[i])[j] = j % 128;
		}
	}

	// Allocate power-of-2 minus 1 sizes (common edge cases)
	void *pow2_ptrs[12];
	for (int i = 0; i < 12; i++) {
		size_t size = (1UL << i) - 1; // 2^i - 1
		pow2_ptrs[i] = malloc(size);
		assert(pow2_ptrs[i] != NULL);

		// Write to the end to ensure full allocation is usable
		if (size > 0) {
			((char *)pow2_ptrs[i])[size - 1] = 0xFF;
		}
	}

	// Free everything
	for (size_t i = 0; i < sizeof(primes) / sizeof(int); i++) {
		free(prime_ptrs[i]);
	}

	for (int i = 0; i < 12; i++) {
		free(pow2_ptrs[i]);
	}

	ft_printf("PASSED: Weird-sized allocations\n");
}

// Test complex nested structures
typedef struct nested_struct {
	char *str;
	int data;
	struct nested_struct *next;
} nested_struct_t;

void test_complex_structures() {
	ft_printf("Testing complex nested structures...\n");

	// Create a linked list with 100 elements, each with its own string
	nested_struct_t *head = NULL;
	nested_struct_t *current = NULL;

	for (int i = 0; i < 100; i++) {
		nested_struct_t *node = (nested_struct_t *)malloc(sizeof(nested_struct_t));
		assert(node != NULL);

		// Allocate string of varying size
		size_t str_len = i + 10;
		node->str = (char *)malloc(str_len);
		assert(node->str != NULL);

		// Fill string with repeating pattern
		for (size_t j = 0; j < str_len - 1; j++) {
			node->str[j] = 'A' + (i % 26);
		}
		node->str[str_len - 1] = '\0';

		node->data = i;
		node->next = NULL;

		if (head == NULL) {
			head = node;
			current = node;
		} else {
			current->next = node;
			current = node;
		}
	}

	// Navigate the list and free everything
	current = head;
	while (current != NULL) {
		nested_struct_t *next = current->next;
		free(current->str);
		free(current);
		current = next;
	}

	ft_printf("PASSED: Complex nested structures\n");
}

// Test strange allocation/free patterns
void test_weird_patterns() {
	ft_printf("Testing weird allocation patterns...\n");

	// Pattern 1: Checkerboard - Allocate 100 blocks, free every other one
	void *checkerboard[100];
	for (int i = 0; i < 100; i++) {
		checkerboard[i] = malloc(128);
		assert(checkerboard[i] != NULL);
	}

	for (int i = 0; i < 100; i += 2) {
		free(checkerboard[i]);
		checkerboard[i] = NULL;
	}

	// Now allocate into the gaps
	for (int i = 0; i < 100; i += 2) {
		checkerboard[i] = malloc(64); // Half the original size
		assert(checkerboard[i] != NULL);
	}

	// Free everything
	for (int i = 0; i < 100; i++) {
		free(checkerboard[i]);
	}

	// Pattern 2: Stack-like allocations and frees
	void *stack[1000];
	for (int i = 0; i < 1000; i++) {
		stack[i] = malloc(i + 1);
		assert(stack[i] != NULL);
	}

	for (int i = 999; i >= 0; i--) {
		free(stack[i]);
	}

	// Pattern 3: Queue-like allocations and frees
	void *queue[1000];
	for (int i = 0; i < 1000; i++) {
		queue[i] = malloc(i + 1);
		assert(queue[i] != NULL);
	}

	for (int i = 0; i < 1000; i++) {
		free(queue[i]);
	}

	ft_printf("PASSED: Weird allocation patterns\n");
}

// Test memory alignment
void test_alignment() {
	ft_printf("Testing memory alignment...\n");

	// Allocate memory for different data types with alignment requirements
	int *int_ptr = (int *)malloc(sizeof(int));
	assert(int_ptr != NULL);
	assert(((uintptr_t)int_ptr % __alignof__(int)) == 0);

	double *double_ptr = (double *)malloc(sizeof(double));
	assert(double_ptr != NULL);
	assert(((uintptr_t)double_ptr % __alignof__(double)) == 0);

	// Test with a 16-byte aligned struct (simulating SSE/AVX types)
	typedef struct {
		char data[32];
	} __attribute__((aligned(16))) aligned_t;

	aligned_t *aligned_ptr = (aligned_t *)malloc(sizeof(aligned_t));
	assert(aligned_ptr != NULL);
	// Check if address is 16-byte aligned
	assert(((uintptr_t)aligned_ptr % 16) == 0);

	free(int_ptr);
	free(double_ptr);
	free(aligned_ptr);

	ft_printf("PASSED: Memory alignment\n");
}

// Random walk allocation pattern
void test_random_walk() {
	ft_printf("Testing random walk allocation pattern...\n");

#define MAX_PTRS 10000
	void *ptrs[MAX_PTRS] = {0};
	int allocated = 0;

	// Simulate a random sequence of allocs and frees
	for (int i = 0; i < 100000; i++) {
		int action = rand() % 100;

		if (action < 60 && allocated < MAX_PTRS) { // 60% chance to allocate
			size_t size = rand() % 1024 + 1;
			ptrs[allocated] = malloc(size);
			if (ptrs[allocated]) {
				// Write something to ensure memory is usable
				*(char *)ptrs[allocated] = 1;
				allocated++;
			}
		} else if (allocated > 0) { // 40% chance to free if we have allocations
			int idx = rand() % allocated;
			free(ptrs[idx]);

			// Move the last pointer to this slot
			ptrs[idx] = ptrs[allocated - 1];
			ptrs[allocated - 1] = NULL;
			allocated--;
		}
	}

	// Clean up any remaining allocations
	for (int i = 0; i < allocated; i++) {
		free(ptrs[i]);
	}

	ft_printf("PASSED: Random walk allocation\n");
}

int main() {
	ft_printf("=== ABSURD MALLOC TESTS ===\n\n");

	// Run the tests
	test_extreme_large_allocations();
	test_tiny_allocations();
	test_weird_sized_allocations();
	test_complex_structures();
	test_weird_patterns();
	test_alignment();
	test_random_walk();

	ft_printf("\nAll absurd tests completed successfully!\n");
	return 0;
}
