#include "malloc.h"
#include "malloc_internal.h"

void init_malloc_system(void) {
	static t_bool initialized = false;

	if (!initialized) {
		pthread_mutex_init(&g_malloc_mutex, NULL);
		initialized = true;
	}
}

size_t get_max_allocation_size(void) {
	static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
	static size_t cached_max_size = 0;

	if (cached_max_size == 0) {
		pthread_mutex_lock(&init_mutex);
		if (cached_max_size != 0) {
			pthread_mutex_unlock(&init_mutex);
			return cached_max_size;
		}
		struct rlimit rl;
		if (getrlimit(RLIMIT_DATA, &rl) == 0)
			cached_max_size = rl.rlim_cur / 2;
		else
			cached_max_size = (1UL << 30); // 1GB fallback
		pthread_mutex_unlock(&init_mutex);
	}
	return cached_max_size;
}

zone_type_t get_zone_type(size_t size) { return GET_ZONE_TYPE(size); }

size_t calculate_needed_size(size_t user_size) {
	return CALC_NEEDED_SIZE(user_size);
}
