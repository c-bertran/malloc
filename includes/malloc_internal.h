#ifndef FT_MALLOC_INTERNAL_H
#define FT_MALLOC_INTERNAL_H
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
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
#define BLOCK_METADATA_SIZE (offsetof(t_block, padding))
#define BLOCK_TOTAL_SIZE(user_size) (ALIGN(BLOCK_METADATA_SIZE + (user_size)))

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
 * @brief Log memory allocation operation
 */
void log(const char *operation, void *ptr, size_t size);

#endif
