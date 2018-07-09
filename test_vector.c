#include <assert.h>
#include <stdio.h>
#include "aqcc.h"

int main()
{
    Vector *vec = new_vector();
    int ch;

    while ((ch = fgetc(stdin)) != EOF) {
        int *d = safe_malloc(sizeof(int));
        *d = ch;
        vector_push_back(vec, d);
    }

    for (int i = 0; i < vec->size; i++) {
        int *d = (int *)vector_get(vec, i);
        printf("%c", (char)*d);
    }
    printf("\n");

    // out-of-range access should return NULL.
    assert(vector_get(vec, vec->size) == NULL);
}
