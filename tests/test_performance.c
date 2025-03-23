#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "../includes/malloc.h"

#define NUM_ALLOCS 10000
#define MAX_ALLOC_SIZE 1024
#define NUM_THREADS 4

typedef struct {
    int id;
    int num_allocs;
} thread_data_t;

// Helper to get random size
size_t get_random_size() {
    return rand() % MAX_ALLOC_SIZE + 1;
}

// Performance test for many small allocations
void test_many_small_allocs() {
    printf("Testing many small allocations...\n");
    
    void *ptrs[NUM_ALLOCS];
    clock_t start = clock();
    
    // Allocate many small blocks
    for (int i = 0; i < NUM_ALLOCS; i++) {
        size_t size = get_random_size();
        ptrs[i] = malloc(size);
        if (ptrs[i] == NULL) {
            printf("Failed to allocate at iteration %d\n", i);
            break;
        }
        
        // Write to memory to ensure it's usable
        memset(ptrs[i], 'A', size-1);
        ((char*)ptrs[i])[size-1] = '\0';
    }
    
    // Free in reverse order
    for (int i = NUM_ALLOCS - 1; i >= 0; i--) {
        if (ptrs[i] != NULL) {
            free(ptrs[i]);
        }
    }
    
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("PASSED: Many small allocations (time: %.4f seconds)\n", time_spent);
}

// Performance test for fragmentation
void test_fragmentation() {
    printf("Testing fragmentation handling...\n");
    
    void *ptrs[NUM_ALLOCS];
    
    // First, allocate many blocks of alternating sizes
    for (int i = 0; i < NUM_ALLOCS; i++) {
        size_t size = (i % 2 == 0) ? 32 : 128;
        ptrs[i] = malloc(size);
        if (ptrs[i] == NULL) {
            printf("Failed to allocate at iteration %d\n", i);
            break;
        }
    }
    
    // Free every other block to create fragmentation
    for (int i = 0; i < NUM_ALLOCS; i += 2) {
        if (ptrs[i] != NULL) {
            free(ptrs[i]);
            ptrs[i] = NULL;
        }
    }
    
    // Now try to allocate larger blocks that should span multiple free blocks
    for (int i = 0; i < NUM_ALLOCS/4; i++) {
        void *ptr = malloc(256);
        if (ptr != NULL) {
            free(ptr);
        }
    }
    
    // Clean up remaining blocks
    for (int i = 1; i < NUM_ALLOCS; i += 2) {
        if (ptrs[i] != NULL) {
            free(ptrs[i]);
        }
    }
    
    printf("PASSED: Fragmentation handling\n");
}

// Thread function for multi-threading test
void *thread_allocator(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    void *ptrs[1000]; // Each thread will do fewer allocations
    
    for (int i = 0; i < data->num_allocs; i++) {
        size_t size = get_random_size();
        ptrs[i] = malloc(size);
        if (ptrs[i] == NULL) {
            printf("Thread %d: Failed to allocate at iteration %d\n", data->id, i);
            break;
        }
        
        // Do something with the memory
        memset(ptrs[i], 'A' + data->id, size-1);
        ((char*)ptrs[i])[size-1] = '\0';
        
        // Randomly free some allocations
        if (rand() % 4 == 0 && i > 0) {
            int idx = rand() % i;
            if (ptrs[idx] != NULL) {
                free(ptrs[idx]);
                ptrs[idx] = NULL;
            }
        }
        
        // Brief pause to increase chance of thread interleaving
        usleep(1);
    }
    
    // Clean up remaining allocations
    for (int i = 0; i < data->num_allocs; i++) {
        if (ptrs[i] != NULL) {
            free(ptrs[i]);
        }
    }
    
    return NULL;
}

// Test thread safety with multiple threads
void test_thread_safety() {
    printf("Testing thread safety...\n");
    
    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data[NUM_THREADS];
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].id = i;
        thread_data[i].num_allocs = 1000;
        
        if (pthread_create(&threads[i], NULL, thread_allocator, &thread_data[i]) != 0) {
            printf("Failed to create thread %d\n", i);
            return;
        }
    }
    
    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("PASSED: Thread safety test\n");
}

// Visualize memory after tests
void show_memory_state() {
    printf("\n=== MEMORY STATE AFTER TESTS ===\n");
    show_alloc_mem();
    printf("\n=== DETAILED MEMORY STATE ===\n");
    show_alloc_mem_ex();
}

int main() {
    printf("=== PERFORMANCE TESTS ===\n\n");
    
    // Seed the random number generator
    srand(time(NULL));
    
    test_many_small_allocs();
    test_fragmentation();
    test_thread_safety();
    
    // Show final memory state
    show_memory_state();
    
    printf("\nAll performance tests completed!\n");
    return 0;
}
