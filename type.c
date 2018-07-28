#include "aqcc.h"

Type *new_type(int kind, int nbytes)
{
    Type *this;

    this = safe_malloc(sizeof(Type));
    this->kind = kind;
    this->nbytes = nbytes;
    return this;
}

Type *type_int()
{
    static Type *type = NULL;
    if (type == NULL) {
        type = new_type(TY_INT, 4);
    }

    return type;
}

Type *new_pointer_type(Type *src)
{
    Type *this;

    this = new_type(TY_PTR, 8);
    this->ptr_of = src;
    return this;
}

Type *new_array_type(Type *src, int len)
{
    Type *this;

    this = new_type(TY_ARY, src->nbytes * len);
    this->ary_of = src;
    this->len = len;
    return this;
}
