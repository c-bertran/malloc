#include "malloc.h"
#include "malloc_internal.h"

bool init_malloc(void) {
    t_zone *tiny_zone = create_zone(ZONE_TINY, 0);
    if (!tiny_zone)
        return false;
    g_zones = tiny_zone;
    return true;
}

t_zone *create_zone(zone_type_t type, size_t size) {
    // Calculate zone size based on type
    size_t zone_size;
    
    if (type == ZONE_TINY) {
        // Size for at least 100 TINY allocations + zone metadata + block headers
        zone_size = (TINY_MAX_SIZE + sizeof(t_block)) * MIN_ALLOC_PER_ZONE + sizeof(t_zone);
    } else if (type == ZONE_SMALL) {
        // Size for at least 100 SMALL allocations + zone metadata + block headers
        zone_size = (SMALL_MAX_SIZE + sizeof(t_block)) * MIN_ALLOC_PER_ZONE + sizeof(t_zone);
    } else { // ZONE_LARGE
        // For large zones, size is just the requested size + metadata
        zone_size = size + sizeof(t_block) + sizeof(t_zone);
    }
    
    // Round up to page size
    size_t page_size = sysconf(_SC_PAGESIZE);
    zone_size = (zone_size + page_size - 1) & ~(page_size - 1);
    
    // Allocate memory with mmap
    void *addr = mmap(NULL, zone_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED)
        return NULL;
        
    // Setup zone metadata
    t_zone *zone = (t_zone *)addr;
    zone->start = addr;
    zone->total_size = zone_size;
    zone->type = type;
    zone->next = NULL;
    zone->free_space = zone_size - sizeof(t_zone);
    zone->used_blocks = 0;
    
    // Create initial free block
    t_block *block = (t_block *)((char *)addr + sizeof(t_zone));
    block->size = zone_size - sizeof(t_zone) - sizeof(t_block);
    block->is_free = true;
    block->next = NULL;
    block->prev = NULL;
    block->magic = MAGIC_NUMBER;
    
    zone->blocks = block;
    
    return zone;
}
