#include <stdlib.h>
#include "aqcc.h"

struct Vector {
    size_t size, rsved_size;
    void **data;
};

Vector *new_vector()
{
    Vector *ret;

    ret = safe_malloc(sizeof(Vector));
    ret->size = 0;
    ret->rsved_size = 1;
    ret->data = NULL;
    return ret;
}

size_t vector_size(Vector *this) { return this->size; }

void vector_push_back(Vector *this, void *item)
{
    if (this->data == NULL || this->size == this->rsved_size) {
        this->rsved_size *= 2;
        this->data =
            safe_realloc(this->data, this->rsved_size * sizeof(void *));
    }

    this->data[this->size++] = item;
}

void *vector_get(Vector *this, size_t i)
{
    if (i >= this->size) return NULL;
    return this->data[i];
}

void *vector_set(Vector *this, size_t i, void *item)
{
    assert(this != NULL && i < vector_size(this));
    this->data[i] = item;
    return item;
}
