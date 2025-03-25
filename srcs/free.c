#include "malloc.h"
#include "malloc_internal.h"

void free(void *ptr) {
	pthread_mutex_lock(&g_malloc_mutex);
	if (!ptr) {
		log("free", NULL, 0);
		pthread_mutex_unlock(&g_malloc_mutex);
		return;
	}

	pthread_mutex_unlock(&g_malloc_mutex);
}
