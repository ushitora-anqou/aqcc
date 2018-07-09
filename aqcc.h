#pragma once
#ifndef AQCC_AQCC_H

#include <stdlib.h>

typedef struct {
    size_t size, rsved_size;
    void **data;
} Vector;

void *safe_malloc(size_t size);
void *safe_realloc(void *ptr, size_t size);

Vector *new_vector();
void vector_push_back(Vector *this, void *item);
void *vector_get(Vector *this, size_t i);

#endif
