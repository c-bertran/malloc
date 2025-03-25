#include "malloc.h"
#include "malloc_internal.h"

// Init global variables
t_zone *g_zones = NULL;
pthread_mutex_t g_malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

void *malloc(size_t size) {
	pthread_mutex_lock(&g_malloc_mutex);
	void *result;
	static bool initialized = false;

	if (size == 0)
		size = 1;
	if (!initialized) {
		if (!init_malloc()) {
			log_operation("malloc", NULL, size);
			pthread_mutex_unlock(&g_malloc_mutex);
			return NULL;
		}
		initialized = true;
	}
	result = internal_allocation_logic(size);
	log_operation("malloc", result, size);
	pthread_mutex_unlock(&g_malloc_mutex);

	return result;
}
