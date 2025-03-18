#include "malloc.h"
#include "malloc_internal.h"

/**
 * @brief Find a suitable free block in a zone
 * @param zone Zone to search
 * @param size Size needed
 * @return Pointer to block header if found, NULL otherwise
 */
static t_block *find_free_block_in_zone(t_zone *zone, size_t size) {
    t_block *current = zone->blocks;
    
    while (current) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

/**
 * @brief Split block if it's significantly larger than needed
 * @param block Block to split
 * @param size Size needed
 * @return true if split was performed, false otherwise
 */
static bool split_if_necessary(t_block *block, size_t size) {
    // Minimum size worth splitting (header size + minimum useful fragment)
    size_t min_split_size = sizeof(t_block) + 16;
    
    // Check if block is big enough to split
    if (block->size >= size + min_split_size) {
        // Create new block after the allocated portion
        t_block *new_block = (t_block *)((char *)block + sizeof(t_block) + size);
        new_block->size = block->size - size - sizeof(t_block);
        new_block->is_free = true;
        new_block->magic = MAGIC_NUMBER;
        
        // Update linked list
        new_block->next = block->next;
        new_block->prev = block;
        if (block->next) {
            block->next->prev = new_block;
        }
        block->next = new_block;
        
        // Update original block size
        block->size = size;
        
        return true;
    }
    
    return false;
}

/**
 * @brief Core allocation function
 * @param size Size requested by user
 * @return Pointer to allocated memory or NULL on failure
 */
void *internal_allocation_logic(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    // Align size to ensure proper memory alignment
    size_t aligned_size = align_size(size);
    
    // Determine zone type based on size
    zone_type_t zone_type = get_zone_type(aligned_size);
    
    // First, try to find a free block in existing zones
    t_zone *current_zone = g_zones;
    t_block *free_block = NULL;
    
    while (current_zone) {
        if (current_zone->type == zone_type) {
            free_block = find_free_block_in_zone(current_zone, aligned_size);
            if (free_block) {
                break;
            }
        }
        current_zone = current_zone->next;
    }
    
    // If no suitable free block found, create a new zone
    if (!free_block) {
        current_zone = create_zone(zone_type, aligned_size);
        if (!current_zone) {
            return NULL; // Failed to create zone
        }
        
        // Add new zone to front of zones list
        current_zone->next = g_zones;
        g_zones = current_zone;
        
        // Get the first block in the new zone
        free_block = current_zone->blocks;
    }
    
    // Mark block as used
    free_block->is_free = false;
    
    // Split block if it's much larger than needed
    split_if_necessary(free_block, aligned_size);
    
    // Update zone metadata
    current_zone->free_space -= free_block->size;
    current_zone->used_blocks++;
    
    // Return pointer to user data area (after header)
    return &(free_block->padding);
}

/**
 * @brief Align size to system word boundary
 * @param size Size to align
 * @return Aligned size
 */
size_t align_size(size_t size) {
    // Align to 16 bytes (common alignment for most architectures)
    size_t alignment = 16;
    return (size + alignment - 1) & ~(alignment - 1);
}

/**
 * @brief Get zone type for a given allocation size
 * @param size Size to allocate
 * @return Zone type (TINY, SMALL, or LARGE)
 */
zone_type_t get_zone_type(size_t size) {
    if (size <= TINY_MAX_SIZE) {
        return ZONE_TINY;
    } else if (size <= SMALL_MAX_SIZE) {
        return ZONE_SMALL;
    } else {
        return ZONE_LARGE;
    }
}
