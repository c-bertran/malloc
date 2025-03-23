#ifndef MALLOC_INTERNAL_H
#define MALLOC_INTERNAL_H
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

/* Debug flag */
#define DEBUG_MALLOC 1

/* Size definitions */
#define TINY_MAX_SIZE 128
#define SMALL_MAX_SIZE 1024

/* Each zone must contain at least 100 allocations */
#define MIN_ALLOC_PER_ZONE 100

/* Magic number (Traditionally used to mark freed memory ("dead" memory)) */
#define MAGIC_NUMBER 0xDEADBEEF

/* Zone types */
typedef enum { ZONE_TINY, ZONE_SMALL, ZONE_LARGE } zone_type_t;

/*
 * Memory block header structure
 * Must be aligned to ensure proper alignment of user data
 */
typedef struct s_block {
	size_t size;          /* Size of the data area */
	bool is_free;         /* Indicates if block is free */
	struct s_block *next; /* Next block in the zone */
	struct s_block *prev; /* Previous block in the zone */
	__uint32_t magic;     /* Magic number for validation */
	/* Padding for alignment */
	char padding[0]; /* Start of user data */
} t_block;

#define BLOCK_HEADER_SIZE ((size_t)(&((t_block *)0)->padding))

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

/* Internal helper functions */

/**
 * @brief Initialize the memory allocator
 * @return true if initialization succeeds, false otherwise
 */
bool init_malloc(void);

/**
 * @brief Get the appropriate zone type for a given size
 * @param size Size in bytes
 * @return The zone type (TINY, SMALL, or LARGE)
 */
zone_type_t get_zone_type(size_t size);

/**
 * @brief Core allocation function
 * @param size Size requested by user
 * @return Pointer to allocated memory or NULL on failure
 */
void *internal_allocation_logic(size_t size);

/**
 * @brief Create a new zone of specified type
 * @param type Zone type
 * @param size Size needed (used for LARGE zones)
 * @return Pointer to new zone, or NULL if allocation fails
 */
t_zone *create_zone(zone_type_t type, size_t size);

/**
 * @brief Find a suitable free block in a zone
 * @param zone Zone to search
 * @param size Size needed
 * @return Pointer to block header if found, NULL otherwise
 */
t_block *find_free_block_in_zone(t_zone *zone, size_t size);

/**
 * @brief Split block if it's significantly larger than needed
 * @param block Block to split
 * @param size Size needed
 * @return true if split was performed, false otherwise
 */
bool split_block(t_block *block, size_t size);

/**
 * @brief Merge adjacent free blocks to reduce fragmentation
 * @param block Starting block to check for merging
 * @return Pointer to the merged block
 */
t_block *merge_blocks(t_block *block);

/**
 * @brief Get the zone containing a given pointer
 * @param ptr Pointer to check
 * @return Zone containing the pointer, or NULL if invalid
 */
t_zone *find_zone_for_ptr(void *ptr);

/**
 * @brief Get the block header for a given user pointer
 * @param ptr User data pointer
 * @return Block header pointer, or NULL if invalid
 */
t_block *get_block_from_ptr(void *ptr);

/**
 * @brief Align size to system word boundary
 * @param size Size to align
 * @return Aligned size
 */
size_t align_size(size_t size);

/**
 * @brief Check if a pointer is valid for deallocation
 * @param ptr Pointer to validate
 * @return true if pointer is valid, false otherwise
 */
bool is_valid_ptr(void *ptr);

/**
 * @brief Log an allocation operation (for debugging)
 * @param operation Operation name (malloc, free, realloc)
 * @param ptr Pointer involved
 * @param size Size involved
 */
void log_operation(const char *operation, void *ptr, size_t size);

#endif
