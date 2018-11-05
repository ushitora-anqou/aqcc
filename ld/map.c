#include "ld.h"

struct KeyValue {
    const char *key;
    void *value;
};

struct Map {
    Vector *data;
};

Map *new_map()
{
    Map *map = safe_malloc(sizeof(Map));
    map->data = new_vector();
    return map;
}

int map_size(Map *map) { return vector_size(map->data); }

KeyValue *map_insert(Map *map, const char *key, void *item)
{
    KeyValue *kv = safe_malloc(sizeof(KeyValue));
    kv->key = key;
    kv->value = item;
    vector_push_back(map->data, kv);
    return kv;
}

KeyValue *map_lookup(Map *map, const char *key)
{
    int i;

    for (i = 0; i < vector_size(map->data); i++) {
        KeyValue *kv = (KeyValue *)vector_get(map->data, i);
        if (strcmp(kv->key, key) == 0) return kv;
    }

    return NULL;
}

const char *kv_key(KeyValue *kv)
{
    if (kv == NULL) return NULL;
    return kv->key;
}

void *kv_value(KeyValue *kv)
{
    if (kv == NULL) return NULL;
    return kv->value;
}
