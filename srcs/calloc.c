#include "malloc.h"
#include "malloc_internal.h"
#include <stdint.h>

void *calloc(size_t nmemb, size_t size) {
    if (nmemb > 0 && size > SIZE_MAX / nmemb)
    {
        log_operation("calloc-overflow", NULL, 0);
        return NULL;
    }
    size_t total_size = nmemb * size;
    void *ptr = malloc(total_size);

    if (ptr == NULL)
    {
        log_operation("calloc-failed", NULL, total_size);
        return NULL;
    }

    unsigned char *byte_ptr = ptr;
    for (size_t i = 0; i < total_size; i++)
        byte_ptr[i] = 0;

    log_operation("calloc", ptr, total_size);
    return ptr;
}
