#include "malloc.h"
#include "malloc_internal.h"

// Init global variables
g_zones = NULL;
g_malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

void *malloc(size_t size) {
	void *result;

	pthread_mutex_lock(&g_malloc_mutex);
	log("malloc", NULL, size);
	if (size == 0)
		size = 1;
	pthread_mutex_unlock(&g_malloc_mutex);
	return result;
}
