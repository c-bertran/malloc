#include "malloc.h"
#include "malloc_internal.h"

void init_malloc_system(void) {
	static bool initialized = false;

	if (!initialized) {
		// Initialize mutex
		pthread_mutex_init(&g_malloc_mutex, NULL);
		initialized = true;
	}
}

zone_type_t get_zone_type(size_t size) { return GET_ZONE_TYPE(size); }

size_t calculate_needed_size(size_t user_size) {
	return CALC_NEEDED_SIZE(user_size);
}
