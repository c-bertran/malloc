#include "malloc.h"
#include "malloc_internal.h"

// Init global variables
t_zone *g_zones = NULL;
pthread_mutex_t g_malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

void *malloc(size_t size) {
	void *result;
  static bool initialized = false;
    
  // Lock the mutex before accessing shared structures
  pthread_mutex_lock(&g_malloc_mutex);
    
  // First-time initialization check
  if (!initialized) {
    if (!init_malloc()) {
      pthread_mutex_unlock(&g_malloc_mutex);
      return NULL;
    }
    initialized = true;
  }
    
  // Perform allocation logic
  // ... find zone, locate free block, update metadata ...
  result = internal_allocation_logic(size);
  
  // Unlock before returning
  pthread_mutex_unlock(&g_malloc_mutex);
    
  return result;
}
