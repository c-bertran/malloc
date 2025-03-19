#include "malloc.h"
#include "malloc_internal.h"

void *realloc(void *ptr, size_t size)
{
    // Case 1: If ptr is NULL, behave like malloc
    if (!ptr) {
        void *ret = malloc(size);
        log_operation("realloc-null", ret, size);
        return ret;
    }
        
    // Case 2: If size is 0, behave like free and return NULL
    if (size == 0) {
        log_operation("realloc-zero", ptr, 0);
        free(ptr);
        return NULL;
    }
    
    pthread_mutex_lock(&g_malloc_mutex);
    
    // Validate the pointer
    t_zone *zone = find_zone_for_ptr(ptr);
    if (!zone) {
        log_operation("realloc-invalid", ptr, size);
        pthread_mutex_unlock(&g_malloc_mutex);
        return NULL;  // Invalid pointer
    }
    
    t_block *block = get_block_from_ptr(ptr);
    if (!block || block->magic != MAGIC_NUMBER || block->is_free) {
        log_operation("realloc-invalid", ptr, size);
        pthread_mutex_unlock(&g_malloc_mutex);
        return NULL;  // Invalid block
    }
    
    // Align the requested size (same as in malloc)
    size_t aligned_size = align_size(size);
    
    // Case 3: Same size, return the same pointer
    if (aligned_size == block->size) {
        log_operation("realloc-same", ptr, size);
        pthread_mutex_unlock(&g_malloc_mutex);
        return ptr;
    }
    
    // Case 4: Smaller size, shrink the block
    if (aligned_size < block->size) {
        // Try to split the block
        split_block(block, aligned_size);
        pthread_mutex_unlock(&g_malloc_mutex);
        return ptr;
    }
    
    // Case 5: Larger size, check if we can expand in-place
    if (block->next && block->next->is_free && 
        (block->size + sizeof(t_block) + block->next->size >= aligned_size)) {
        
        // Merge with next block
        size_t combined_size = block->size + sizeof(t_block) + block->next->size;
        block->size = combined_size;
        block->next = block->next->next;
        if (block->next)
            block->next->prev = block;
            
        // If resulting block is much larger, split it
        split_block(block, aligned_size);
        
        pthread_mutex_unlock(&g_malloc_mutex);
        return ptr;
    }
    
    // Case 6: Need to relocate, can't expand in-place
    pthread_mutex_unlock(&g_malloc_mutex);
    
    // Allocate new block
    void *new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;
        
    // Copy data from old block to new block (smaller of the two sizes)
    size_t copy_size = (block->size < size) ? block->size : size;
    
    // Copy memory manually without memcpy (since you can only use allowed functions)
    char *src = ptr;
    char *dst = new_ptr;
    for (size_t i = 0; i < copy_size; i++)
        dst[i] = src[i];
    
    // Free old block
    free(ptr);
    
    log_operation("realloc", new_ptr, size);

    return new_ptr;
}