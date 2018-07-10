#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aqcc.h"

void error(const char *msg, const char *filename, int lineno)
{
    fprintf(stderr, "%s: %s, %d\n", msg, filename, lineno);
    exit(EXIT_FAILURE);
}

void *safe_malloc(size_t size)
{
    void *ptr;

    ptr = malloc(size);
    if (ptr == NULL) error("malloc failed.", __FILE__, __LINE__);
    return ptr;
}

void *safe_realloc(void *ptr, size_t size)
{
    ptr = realloc(ptr, size);
    if (ptr == NULL) error("realloc failed.", __FILE__, __LINE__);
    return ptr;
}

char *new_str(const char *src)
{
    char *ret = safe_malloc(strlen(src) + 1);
    strcpy(ret, src);
    return ret;
}
