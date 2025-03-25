#ifndef FT_MALLOC_INTERNAL_H
#define FT_MALLOC_INTERNAL_H
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

#ifndef DEBUG_MALLOC
/**
 * @brief Enable debug output for memory allocation operations
 * @note Set to 1 to activate debug output
 */
#define DEBUG_MALLOC 0
#endif

/* Tiny size */
#define TINY_MAX_SIZE 128
/* Small size */
#define SMALL_MAX_SIZE 1024
/* Minimum allocation per zone */
#define MIN_ALLOC_PER_ZONE 100
/* Magic number of freed memory */
#define MAGIC_NUMBER 0xDEADBEEF

/* Standard 16-byte memory alignment */
#define MALLOC_ALIGNMENT 16
/* Macro to align size up to alignment boundary */
#define ALIGN(size)                                                            \
	(((size) + (MALLOC_ALIGNMENT - 1)) & ~(MALLOC_ALIGNMENT - 1))
/* Page size using sysconf(_SC_PAGESIZE) for Linux */
#define PAGE_SIZE (sysconf(_SC_PAGESIZE))
/* Zone size calculations for pre-allocation */
#define TINY_ZONE_SIZE                                                         \
	(PAGE_SIZE * ((TINY_MAX_SIZE * MIN_ALLOC_PER_ZONE) / PAGE_SIZE + 1))
#define SMALL_ZONE_SIZE                                                        \
	(PAGE_SIZE * ((SMALL_MAX_SIZE * MIN_ALLOC_PER_ZONE) / PAGE_SIZE + 1))
/* Block size calculations */
#define BLOCK_METADATA_SIZE                                                    \
	(sizeof(struct s_block *) + sizeof(struct s_block *) + sizeof(size_t) +      \
	 sizeof(uint32_t) + sizeof(bool) + sizeof(size_t))
#define BLOCK_TOTAL_SIZE(user_size) (ALIGN(BLOCK_METADATA_SIZE + (user_size)))

/* Get appropriate zone type for allocation size */
#define GET_ZONE_TYPE(size)                                                    \
	((size <= TINY_MAX_SIZE)    ? ZONE_TINY                                      \
	 : (size <= SMALL_MAX_SIZE) ? ZONE_SMALL                                     \
	                            : ZONE_LARGE)

/* Calculate total size needed for a user allocation */
#define CALC_NEEDED_SIZE(user_size) (BLOCK_TOTAL_SIZE(user_size))

/* Zone types */
typedef enum { ZONE_TINY, ZONE_SMALL, ZONE_LARGE } zone_type_t;

/*
 * Memory block header structure
 * Must be aligned to ensure proper alignment of user data
 */
typedef struct s_block {
	struct s_block *next; /* Next block in the zone */
	struct s_block *prev; /* Previous block in the zone */
	size_t size;          /* Size of the data area */
	uint32_t magic;       /* Magic number for validation */
	bool is_free;         /* Indicates if block is free */
	size_t offset;        /* Offset from start of zone */
	char padding[0];      /* Start of user data */
} t_block;

/*
 * Zone structure - manages a contiguous memory region
 */
typedef struct s_zone {
	void *start;         /* Start address of zone memory */
	size_t total_size;   /* Total zone size in bytes */
	zone_type_t type;    /* Zone type (TINY, SMALL, LARGE) */
	struct s_zone *next; /* Next zone in list */
	size_t free_space;   /* Available space in zone */
	size_t used_blocks;  /* Number of allocated blocks */
	t_block *blocks;     /* Pointer to first block in zone */
} t_zone;

/* Global variables */
extern t_zone *g_zones;                /* Head of zones list */
extern pthread_mutex_t g_malloc_mutex; /* Mutex for thread safety */

/**
 * Log memory allocation operation
 */
void logger(const char *operation, void *ptr, size_t size);

// Zone management functions
/**
 * Create a new zone of specified type and size
 */
t_zone *create_zone(zone_type_t type, size_t size);

/**
 * Find appropriate zone for an allocation
 */
t_zone *find_zone_for_size(size_t size);

/**
 * Find a zone containing a specific pointer
 */
t_zone *find_zone_containing(void *ptr);

// Defragmentation functions
/**
 * Calculate fragmentation metrics for a zone
 * Returns a fragmentation score (higher = more fragmented)
 */
float calculate_fragmentation(t_zone *zone);
/**
 * Defragment memory by consolidating free blocks
 * Returns number of zones defragmented
 */
int defragment_memory(void);

// Block management functions
/**
 * Find a free block in a zone that can accommodate size
 */
t_block *find_free_block(t_zone *zone, size_t size);

/**
 * Split a block if it's too large for the requested size
 */
t_block *split_block(t_block *block, size_t size);

/**
 * Merge adjacent free blocks (coalescing)
 */
t_block *merge_blocks(t_block *block);

/**
 * Initialize a new block in a zone
 */
t_block *init_block(t_zone *zone, size_t offset, size_t size);

/**
 * Custom memcpy implementation to copy memory between blocks
 */
void *block_memcpy(void *dest, const void *src, size_t n);

/**
 * Custom memset implementation to set memory in a block
 */
void *block_memset(void *s, int c, size_t n);

// Memory system functions
/**
 * Initialize the memory allocation system
 */
void init_malloc_system(void);

/**
 * Get appropriate zone type for allocation size
 */
zone_type_t get_zone_type(size_t size);

/**
 * Calculate total size needed for a user allocation
 */
size_t calculate_needed_size(size_t user_size);

// Memory validation functions
/**
 * Check if a pointer is a valid allocated block
 */
bool is_valid_pointer(void *ptr);

/**
 * Verify block integrity (check magic number, etc.)
 */
bool verify_block(t_block *block);

// Memory
#endif
