#include <stdlib.h>
#include <string.h>
#include "aqcc.h"

Map *new_map()
{
    Map *this;

    this = safe_malloc(sizeof(Map));
    this->data = new_vector();
    return this;
}

size_t map_size(Map *this) { return vector_size(this->data); }

void map_insert(Map *this, const char *key, void *item)
{
    KeyValue *kv = safe_malloc(sizeof(KeyValue));
    kv->key = new_str(key);
    kv->value = item;
    vector_push_back(this->data, kv);
}

KeyValue *map_lookup(Map *this, const char *key)
{
    size_t i;

    for (i = 0; i < vector_size(this->data); i++) {
        KeyValue *kv = (KeyValue *)vector_get(this->data, i);
        if (strcmp(kv->key, key) == 0) return kv;
    }

    return NULL;
}
