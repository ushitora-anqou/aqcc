#include <stdlib.h>
#include <string.h>
#include "aqcc.h"

struct KeyValue {
    const char *key;
    void *value;
};

struct Map {
    Vector *data;
};

Map *new_map()
{
    Map *this;

    this = safe_malloc(sizeof(Map));
    this->data = new_vector();
    return this;
}

int map_size(Map *this) { return vector_size(this->data); }

KeyValue *map_insert(Map *this, const char *key, void *item)
{
    KeyValue *kv = safe_malloc(sizeof(KeyValue));
    kv->key = key;
    kv->value = item;
    vector_push_back(this->data, kv);
    return kv;
}

KeyValue *map_lookup(Map *this, const char *key)
{
    int i;

    for (i = 0; i < vector_size(this->data); i++) {
        KeyValue *kv = (KeyValue *)vector_get(this->data, i);
        if (strcmp(kv->key, key) == 0) return kv;
    }

    return NULL;
}

const char *kv_key(KeyValue *kv) { return kv->key; }

void *kv_value(KeyValue *kv) { return kv->value; }
