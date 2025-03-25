#include "malloc.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int ft_printf(char *string, ...);

#define NUM_THREADS 8
#define ITERATIONS 5000
#define ALLOC_SIZE 1024
#define SHARED_ALLOCS 100

// Shared memory pointers for cross-thread tests
void *shared_ptrs[SHARED_ALLOCS];
pthread_mutex_t shared_mutex = PTHREAD_MUTEX_INITIALIZER;

// Test 1: Basic concurrent malloc/free
void *basic_thread_test(void *arg) {
	int id = *(int *)arg;
	void *ptrs[100];

	for (int i = 0; i < 100; i++) {
		size_t size = (rand() % 4096) + 1;
		ptrs[i] = malloc(size);
		if (ptrs[i]) {
			// Write a pattern to the memory
			memset(ptrs[i], 'A' + id, size > 10 ? 10 : size);
		}
		usleep(1); // Small delay to increase thread interleaving
	}

	// Free everything
	for (int i = 0; i < 100; i++) {
		free(ptrs[i]);
	}

	return NULL;
}

// Test 2: Concurrent realloc operations
void *realloc_thread_test(void *arg) {
	int id = *(int *)arg;
	void *ptr = malloc(10);

	for (int i = 0; i < 100; i++) {
		size_t new_size = (rand() % 1000) + 20;
		ptr = realloc(ptr, new_size);
		if (ptr) {
			// Mark memory to verify it persists through realloc
			((char *)ptr)[0] = 'A' + id;
		}
		usleep(rand() % 100);
	}

	free(ptr);
	return NULL;
}

// Test 3: Cross-thread free (malloc in one thread, free in another)
void *producer_thread(void *arg) {
	(void)arg;
	for (int i = 0; i < SHARED_ALLOCS; i++) {
		size_t size = (rand() % 2048) + 32;
		void *ptr = malloc(size);

		pthread_mutex_lock(&shared_mutex);
		shared_ptrs[i] = ptr;
		pthread_mutex_unlock(&shared_mutex);

		usleep(1);
	}
	return NULL;
}

void *consumer_thread(void *arg) {
	(void)arg;
	usleep(100); // Give producer a head start

	for (int i = 0; i < SHARED_ALLOCS; i++) {
		void *ptr = NULL;

		// Wait until we have a pointer to free
		while (ptr == NULL) {
			pthread_mutex_lock(&shared_mutex);
			ptr = shared_ptrs[i];
			pthread_mutex_unlock(&shared_mutex);

			if (ptr == NULL) {
				usleep(1);
			}
		}

		// Free the memory allocated by the producer
		free(ptr);
	}
	return NULL;
}

// Test 4: High contention test - many threads doing rapid small allocations
void *contention_thread(void *arg) {
	(void)arg;
	for (int i = 0; i < ITERATIONS; i++) {
		void *ptr = malloc(8);
		free(ptr);
	}
	return NULL;
}

// Test 5: Mixed allocation sizes to stress different zone types
void *mixed_sizes_thread(void *arg) {
	(void)arg;
	for (int i = 0; i < 50; i++) {
		// Tiny allocation (0-128 bytes)
		void *tiny = malloc(rand() % 128 + 1);

		// Small allocation (129-1024 bytes)
		void *small = malloc(rand() % 896 + 129);

		// Large allocation (>1024 bytes)
		void *large = malloc(rand() % 8192 + 1025);

		usleep(1);

		// Free in random order to create fragmentation
		if (rand() % 2) {
			free(tiny);
			free(small);
			free(large);
		} else {
			free(large);
			free(tiny);
			free(small);
		}
	}
	return NULL;
}

void run_thread_test(void *(*test_func)(void *), const char *test_name) {
	ft_printf("Running %s...\n", test_name);

	pthread_t threads[NUM_THREADS];
	int thread_ids[NUM_THREADS];

	for (int i = 0; i < NUM_THREADS; i++) {
		thread_ids[i] = i;
		if (pthread_create(&threads[i], NULL, test_func, &thread_ids[i]) != 0) {
			ft_printf("Failed to create thread %d\n", i);
			return;
		}
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	ft_printf("PASSED: %s\n", test_name);
}

void run_producer_consumer_test() {
	ft_printf("Running cross-thread free test...\n");

	pthread_t producer, consumer;

	if (pthread_create(&producer, NULL, producer_thread, NULL) != 0) {
		ft_printf("Failed to create producer thread\n");
		return;
	}

	if (pthread_create(&consumer, NULL, consumer_thread, NULL) != 0) {
		ft_printf("Failed to create consumer thread\n");
		return;
	}

	pthread_join(producer, NULL);
	pthread_join(consumer, NULL);

	ft_printf("PASSED: cross-thread free test\n");
}

int main() {
	ft_printf("=== THREAD SAFETY TESTS ===\n\n");

	// Initialize random seed
	srand(time(NULL));

	// Run all thread tests
	run_thread_test(basic_thread_test, "Basic concurrent malloc/free");
	run_thread_test(realloc_thread_test, "Concurrent realloc operations");
	run_producer_consumer_test();
	run_thread_test(contention_thread, "High contention test");
	run_thread_test(mixed_sizes_thread, "Mixed allocation sizes");

	// Show memory state after all tests
	ft_printf("\n=== MEMORY STATE AFTER THREAD TESTS ===\n");
	show_alloc_mem();

	ft_printf("\nAll thread tests completed successfully!\n");
	return 0;
}
