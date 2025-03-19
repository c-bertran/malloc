#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../includes/malloc.h"

// Test basic malloc and free
void test_simple_malloc_free() {
    printf("Testing simple malloc and free...\n");
    
    // Allocate memory
    int *ptr = (int *)malloc(sizeof(int) * 10);
    assert(ptr != NULL);
    
    // Write to memory to ensure it's usable
    for (int i = 0; i < 10; i++) {
        ptr[i] = i * 42;
    }
    
    // Read back values to ensure memory integrity
    for (int i = 0; i < 10; i++) {
        assert(ptr[i] == i * 42);
    }
    
    // Free memory
    free(ptr);
    printf("PASSED: Simple malloc and free\n");
}

// Test calloc initialization
void test_calloc() {
    printf("Testing calloc...\n");
    
    // Allocate memory with calloc
    int *ptr = (int *)calloc(10, sizeof(int));
    assert(ptr != NULL);
    
    // Verify memory is initialized to zero
    for (int i = 0; i < 10; i++) {
        assert(ptr[i] == 0);
    }
    
    // Free memory
    free(ptr);
    printf("PASSED: Calloc initialization\n");
}

// Test realloc functionality
void test_realloc() {
    printf("Testing realloc...\n");
    
    // Initial allocation
    int *ptr = (int *)malloc(sizeof(int) * 5);
    assert(ptr != NULL);
    
    // Fill with data
    for (int i = 0; i < 5; i++) {
        ptr[i] = i * 100;
    }
    
    // Expand allocation
    int *new_ptr = (int *)realloc(ptr, sizeof(int) * 10);
    assert(new_ptr != NULL);
    
    // Verify original data is preserved
    for (int i = 0; i < 5; i++) {
        assert(new_ptr[i] == i * 100);
    }
    
    // Add more data
    for (int i = 5; i < 10; i++) {
        new_ptr[i] = i * 100;
    }
    
    // Shrink allocation
    new_ptr = (int *)realloc(new_ptr, sizeof(int) * 3);
    assert(new_ptr != NULL);
    
    // Verify first 3 values are preserved
    for (int i = 0; i < 3; i++) {
        assert(new_ptr[i] == i * 100);
    }
    
    // Free memory
    free(new_ptr);
    printf("PASSED: Realloc functionality\n");
}

// Test different allocation sizes
void test_allocation_sizes() {
    printf("Testing various allocation sizes...\n");
    
    // Test tiny allocations (< 128 bytes)
    char *tiny = (char *)malloc(64);
    assert(tiny != NULL);
    strcpy(tiny, "Tiny allocation test");
    free(tiny);
    
    // Test small allocations (128-1024 bytes)
    char *small = (char *)malloc(512);
    assert(small != NULL);
    memset(small, 'A', 511);
    small[511] = '\0';
    free(small);
    
    // Test large allocations (>1024 bytes)
    char *large = (char *)malloc(2048);
    assert(large != NULL);
    memset(large, 'B', 2047);
    large[2047] = '\0';
    free(large);
    
    printf("PASSED: Various allocation sizes\n");
}

int main() {
    printf("=== BASIC MALLOC TESTS ===\n\n");
    
    test_simple_malloc_free();
    test_calloc();
    test_realloc();
    test_allocation_sizes();
    
    printf("\nAll basic tests passed!\n");
    return 0;
}