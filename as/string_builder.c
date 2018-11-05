#include "as.h"

struct StringBuilder {
    Vector *data;
};

StringBuilder *new_string_builder()
{
    StringBuilder *sb = safe_malloc(sizeof(StringBuilder));
    sb->data = new_vector();
    return sb;
}

char string_builder_append(StringBuilder *sb, char ch)
{
    vector_push_back(sb->data, (void *)ch);
    return ch;
}

char *string_builder_get(StringBuilder *sb)
{
    int size = vector_size(sb->data);
    char *ret = safe_malloc(size + 1);
    for (int i = 0; i < size; i++) ret[i] = (char)vector_get(sb->data, i);
    ret[size] = '\0';
    return ret;
}

int string_builder_size(StringBuilder *sb) { return vector_size(sb->data) + 1; }
