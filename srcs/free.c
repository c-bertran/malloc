#include "malloc.h"
#include "malloc_internal.h"

void free(void *ptr)
{
    if (!ptr)
        return;
    pthread_mutex_lock(&g_malloc_mutex);
    t_zone *zone = find_zone_for_ptr(ptr);
    if (!zone)
    {
        pthread_mutex_unlock(&g_malloc_mutex);
        return;
    }
    t_block *block = get_block_from_ptr(ptr);
    if (!block || block->magic != MAGIC_NUMBER || block->is_free)
    {
        pthread_mutex_unlock(&g_malloc_mutex);
        return;
    }
    // Handle large allocations differently
    if (zone->type == ZONE_LARGE) {
        // Remove from zone list
        t_zone *current = g_zones;
        t_zone *prev = NULL;
        
        while (current && current != zone) {
            prev = current;
            current = current->next;
        }
        
        if (prev)
            prev->next = zone->next;
        else
            g_zones = zone->next;

        // Free the entire zone
        munmap(zone, zone->total_size);
    } else {
        // For TINY and SMALL zones
        block->is_free = true;
        zone->free_space += block->size;
        zone->used_blocks--;
        
        // Try to merge with adjacent blocks
        merge_adjacent_blocks(block);  // No zone parameter needed
    }
    
    pthread_mutex_unlock(&g_malloc_mutex);
}
