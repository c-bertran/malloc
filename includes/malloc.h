#ifndef FT_MALLOC_H
#define FT_MALLOC_H

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

/**
 * @brief Allocates memory of the specified size
 *
 * This function allocates 'size' bytes of memory and returns a pointer
 * to the allocated memory. The memory is not initialized.
 *
 * @param size Size in bytes to allocate
 * @return void* Pointer to allocated memory, or NULL if allocation fails
 */
void *malloc(size_t size);

/**
 * @brief Frees previously allocated memory
 *
 * This function deallocates the memory allocation pointed to by ptr.
 * If ptr is NULL, no operation is performed.
 *
 * @param ptr Pointer to memory to free
 */
void free(void *ptr);

/**
 * @brief Changes the size of a previously allocated memory block
 *
 * This function tries to change the size of the allocation pointed to by ptr
 * to size bytes and returns ptr. If there is not enough room to enlarge the 
 * memory allocation, a new allocation is created, data is copied, the old 
 * allocation is freed, and a pointer to the new allocation is returned.
 *
 * @param ptr Pointer to previously allocated memory
 * @param size New size in bytes
 * @return void* Pointer to the reallocated memory
 */
void *realloc(void *ptr, size_t size);

/**
 * @brief Allocates memory for an array and initializes it to zero
 *
 * This function allocates memory for an array of nmemb elements of size bytes each
 * and returns a pointer to the allocated memory. The memory is set to zero.
 *
 * @param nmemb Number of elements
 * @param size Size in bytes of each element
 * @return void* Pointer to the allocated memory
 */
void *calloc(size_t nmemb, size_t size);

/**
 * @brief Displays information about allocated memory
 *
 * This function prints the state of allocated memory:
 * - Address ranges of allocated memory blocks
 * - Size of each block
 * - Total allocated size
 */
void show_alloc_mem(void);

/**
 * @brief Extended version of show_alloc_mem with more details (bonus)
 *
 * This function displays detailed information about memory allocations:
 * - Memory maps
 * - Allocation history
 * - Hex dumps of allocated zones
 * - Additional statistics
 */
void show_alloc_mem_ex(void);

#endif /* FT_MALLOC_H */
