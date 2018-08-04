#include "aqcc.h"

struct StringBuilder {
    Vector *data;
};

StringBuilder *new_string_builder()
{
    StringBuilder *this;

    this = safe_malloc(sizeof(StringBuilder));
    this->data = new_vector();
    return this;
}

char string_builder_append(StringBuilder *this, char ch)
{
    vector_push_back(this->data, (void *)ch);
    return ch;
}

char *string_builder_get(StringBuilder *this)
{
    char *ret;
    int i, size;

    size = vector_size(this->data);
    ret = safe_malloc(size + 1);
    for (i = 0; i < size; i++) ret[i] = (char)vector_get(this->data, i);
    ret[size] = '\0';
    return ret;
}

size_t string_builder_size(StringBuilder *this)
{
    return vector_size(this->data) + 1;
}
