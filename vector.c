#include <stdlib.h>
#include "aqcc.h"

struct Vector {
    int size, rsved_size;
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

Vector *new_vector_from_scalar(void *scalar)
{
    Vector *this = new_vector();
    vector_push_back(this, scalar);
    return this;
}

int vector_size(Vector *this) { return this->size; }

void vector_push_back(Vector *this, void *item)
{
    if (this->data == NULL || this->size == this->rsved_size) {
        this->rsved_size *= 2;
        this->data =
            safe_realloc(this->data, this->rsved_size * sizeof(void *));
    }

    this->data[this->size++] = item;
}

void *vector_get(Vector *this, int i)
{
    if (i >= this->size) return NULL;
    return this->data[i];
}

void *vector_set(Vector *this, int i, void *item)
{
    assert(this != NULL && i < vector_size(this));
    this->data[i] = item;
    return item;
}

void vector_push_back_vector(Vector *this, Vector *src)
{
    for (int i = 0; i < vector_size(src); i++)
        vector_push_back(this, vector_get(src, i));
}

Vector *clone_vector(Vector *src)
{
    Vector *vec = new_vector();
    vector_push_back_vector(vec, src);
    return vec;
}
