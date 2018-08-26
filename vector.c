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
    Vector *vec = new_vector();
    vector_push_back(vec, scalar);
    return vec;
}

int vector_size(Vector *vec) { return vec->size; }

void vector_push_back(Vector *vec, void *item)
{
    if (vec->data == NULL || vec->size == vec->rsved_size) {
        vec->rsved_size *= 2;
        vec->data = safe_realloc(vec->data, vec->rsved_size * sizeof(void *));
    }

    vec->data[vec->size++] = item;
}

void *vector_get(Vector *vec, int i)
{
    if (i >= vec->size) return NULL;
    return vec->data[i];
}

void *vector_set(Vector *vec, int i, void *item)
{
    assert(vec != NULL && i < vector_size(vec));
    vec->data[i] = item;
    return item;
}

void vector_push_back_vector(Vector *vec, Vector *src)
{
    for (int i = 0; i < vector_size(src); i++)
        vector_push_back(vec, vector_get(src, i));
}

Vector *clone_vector(Vector *src)
{
    Vector *vec = new_vector();
    vector_push_back_vector(vec, src);
    return vec;
}
