#include "malloc.h"
#include "malloc_internal.h"

// Get page size using allowed function
size_t get_page_size(void) {
    return sysconf(_SC_PAGESIZE);
}

// Find zone containing pointer without using forbidden functions
t_zone *find_zone_for_ptr(void *ptr) {
    t_zone *current = g_zones;
    
    while (current) {
        // Check if ptr falls within this zone
        if (ptr >= current->start && 
            ptr < (void *)((char *)current->start + current->total_size)) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Calculate block address from user pointer
t_block *get_block_from_ptr(void *ptr) {
    // User pointer is at block->padding
    size_t padding_offset = sizeof(t_block);
    return (t_block *)((char *)ptr - padding_offset);
}

bool is_valid_ptr(void *ptr) {
    t_zone *zone = find_zone_for_ptr(ptr);
    if (!zone)
        return false;
        
    t_block *block = get_block_from_ptr(ptr);
    
    // Verify block is within zone bounds
    if ((void *)block < zone->start || 
        (void *)block >= (void *)((char *)zone->start + zone->total_size))
        return false;
        
    // Verify magic number
    if (block->magic != MAGIC_NUMBER)
        return false;
        
    // Verify not already freed
    if (block->is_free)
        return false;
        
    return true;
}