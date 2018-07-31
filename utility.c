#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aqcc.h"

_Noreturn void error(const char *msg, const char *filename, int lineno)
{
    fprintf(stderr, "[ERROR] %s: %s, %d\n", msg, filename, lineno);
    exit(EXIT_FAILURE);
}

void warn(const char *msg, const char *filename, int lineno)
{
    fprintf(stderr, "[WARN] %s: %s, %d\n", msg, filename, lineno);
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

char byte2suffix(int byte)
{
    switch (byte) {
        case 8:
            return 'q';
        case 4:
            return 'l';
        default:
            assert(0);
    }
}

int max(int a, int b) { return a > b ? a : b; }
