#include <assert.h>
#include <string.h>

void test_vector(int n)
{
    Vector *vec = new_vector();
    int i = 0;

    for (i = 0; i < n; i++) {
        int *d = safe_malloc(sizeof(int));
        *d = i;
        vector_push_back(vec, d);
    }

    assert(vector_size(vec) == n);

    for (i = 0; i < n; i++) {
        int *d = (int *)vector_get(vec, i);
        vector_push_back(vec, d);
        assert(*d == i);
    }

    vector_set(vec, n - 1, new_int(1));
    assert(*(int*)vector_get(vec, n - 1) == 1);

    // out-of-range access should return NULL.
    assert(vector_get(vec, vector_size(vec)) == NULL);
}

void test_map()
{
    Map *map = new_map();
    const char *key[] = {"key0", "key1", "key2", "key3"};
    int data[] = {0, 1, 2, 3};

    for (int i = 0; i < 4; i++) {
        KeyValue *kv = map_insert(map, key[i], &data[i]);
        assert(strcmp(kv_key(kv), key[i]) == 0);
        assert(*(int *)(kv_value(kv)) == data[i]);
    }

    assert(map_size(map) == sizeof(data) / sizeof(int));

    for (int i = 0; i < 4; i++) {
        KeyValue *kv = map_lookup(map, key[i]);
        assert(strcmp(kv_key(kv), key[i]) == 0);
        assert(*(int *)kv_value(kv) == data[i]);
    }
}

void execute_test()
{
    test_vector(10);
    test_map();
}
