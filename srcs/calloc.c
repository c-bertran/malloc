#include "malloc.h"
#include "malloc_internal.h"

void *calloc(size_t nmemb, size_t size) {
	if (size != 0 && nmemb > SIZE_MAX / size)
		return NULL;

	size_t total_size = nmemb * size;
	void *ptr = malloc(total_size);
	if (ptr)
		block_memset(ptr, 0, total_size);
	return ptr;
}
