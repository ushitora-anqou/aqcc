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

int *new_int(int src)
{
    int *ret = safe_malloc(sizeof(int));
    *ret = src;
    return ret;
}

const char *reg_name(int byte, int i)
{
    const char *rreg[] = {"%rax", "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    const char *ereg[] = {"%eax", "%edi", "%esi", "%edx",
                          "%ecx", "%r8d", "%r9d"};

    assert(0 <= i && i <= 6);

    switch (byte) {
        case 4:
            return ereg[i];
        case 8:
            return rreg[i];
        default:
            assert(0);
    }
}

int type2byte(Type *type)
{
    switch (type->kind) {
        case TY_INT:
            return 4;
        case TY_PTR:
            return 8;
        default:
            assert(0);
    }
}
