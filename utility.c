#include <assert.h>
#include <stdlib.h>
#include "aqcc.h"

void *safe_malloc(size_t size)
{
    void *ptr;

    ptr = malloc(size);
    assert(ptr != NULL);
    return ptr;
}

void *safe_realloc(void *ptr, size_t size)
{
    ptr = realloc(ptr, size);
    assert(ptr != NULL);
    return ptr;
}
