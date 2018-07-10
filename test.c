#include <assert.h>

void test_vector(int n)
{
    Vector *vec = new_vector();
    int i = 0;

    for (i = 0; i < n; i++) {
        int *d = safe_malloc(sizeof(int));
        *d = i;
        vector_push_back(vec, d);
    }

    assert(vec->size == n);

    for (i = 0; i < n; i++) {
        int *d = (int *)vector_get(vec, i);
        vector_push_back(vec, d);
        assert(*d == i);
    }

    // out-of-range access should return NULL.
    assert(vector_get(vec, vec->size) == NULL);
}

void execute_test() { test_vector(10); }
