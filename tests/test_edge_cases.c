#include <stdio.h>
#include <assert.h>
#include "../includes/malloc.h"

// Test NULL pointer handling
void test_null_pointers() {
    printf("Testing NULL pointer handling...\n");
    
    // Free NULL should do nothing
    free(NULL);
    
    // Realloc with NULL should behave like malloc
    void *ptr = realloc(NULL, 10);
    assert(ptr != NULL);
    free(ptr);
    
    printf("PASSED: NULL pointer handling\n");
}

// Test zero-size allocations
void test_zero_size() {
    printf("Testing zero-size allocations...\n");
    
    // malloc(0) should return NULL or a small allocation
    void *ptr = malloc(0);
    free(ptr); // Should work regardless of what malloc(0) returned
    
    // calloc(0, x) should behave like malloc(0)
    ptr = calloc(0, 10);
    free(ptr);
    
    // calloc(x, 0) should behave like malloc(0)
    ptr = calloc(10, 0);
    free(ptr);
    
    // realloc to 0 should free the pointer
    ptr = malloc(10);
    assert(ptr != NULL);
    void *new_ptr = realloc(ptr, 0);
    assert(new_ptr == NULL);
    
    printf("PASSED: Zero-size allocations\n");
}

// Test very large allocations
void test_large_allocations() {
    printf("Testing large allocations...\n");
    
    // Try a very large allocation (will likely fail, but shouldn't crash)
    void *large_ptr = malloc(1024 * 1024 * 1024); // 1GB
    
    if (large_ptr != NULL) {
        // If it succeeded, we should be able to write to it
        char *ptr = (char *)large_ptr;
        ptr[0] = 'A';
        ptr[1024 * 1024 * 1024 - 1] = 'Z';
        free(large_ptr);
        printf("PASSED: Large allocation (1GB)\n");
    } else {
        printf("SKIPPED: Large allocation (1GB) - insufficient memory\n");
    }
    
    // More reasonable large allocation
    void *medium_ptr = malloc(10 * 1024 * 1024); // 10MB
    if (medium_ptr != NULL) {
        free(medium_ptr);
        printf("PASSED: Medium large allocation (10MB)\n");
    } else {
        printf("FAILED: Medium large allocation (10MB)\n");
    }
}

// Test double free detection (should not crash)
void test_double_free() {
    printf("Testing double free behavior...\n");
   
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wuse-after-free"
    {
        void *ptr = malloc(10);
        assert(ptr != NULL);

        free(ptr);  // Valid free
        // This second free should be detected and ignored
        // Should not crash, but might print an error
        free(ptr);
    }
    #pragma GCC diagnostic pop
    
    printf("PASSED: Double free handling\n");
}

// Test invalid pointer to free
void test_invalid_free() {
    printf("Testing invalid free behavior...\n");
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfree-nonheap-object"
    {
        char stack_var[10];
        void *invalid_ptr = stack_var;
        // This free should be detected and ignored
         // Should not crash, but might print an error
        free(invalid_ptr);
    }
    #pragma GCC diagnostic pop
    
    // Create a pointer that's in the middle of an allocation
    void *ptr = malloc(100);
    assert(ptr != NULL);
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfree-nonheap-object"
    {
        void *offset_ptr = (char*)ptr + 10;
        free(offset_ptr);  // Should be detected as invalid
    }
    #pragma GCC diagnostic pop
    
    // Clean up valid pointer
    free(ptr);
    
    printf("PASSED: Invalid free handling\n");
}

int main() {
    printf("=== EDGE CASE TESTS ===\n\n");
    
    test_null_pointers();
    test_zero_size();
    test_large_allocations();
    test_double_free();
    test_invalid_free();
    
    printf("\nAll edge case tests passed!\n");
    return 0;
}
