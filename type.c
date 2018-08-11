#include "aqcc.h"

Type *new_type(int kind, int nbytes)
{
    Type *this;

    this = safe_malloc(sizeof(Type));
    this->kind = kind;
    this->nbytes = nbytes;
    this->is_static = 0;
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

Type *type_char()
{
    static Type *type = NULL;
    if (type == NULL) {
        type = new_type(TY_CHAR, 1);
    }

    return type;
}

Type *type_void()
{
    static Type *type = NULL;
    if (type == NULL) {
        type = new_type(TY_VOID, -1);
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

Type *new_struct_or_union_type(int kind, char *stname, Vector *decls)
{
    Type *this = new_type(kind, -1);
    this->stname = stname;
    this->members = NULL;
    this->decls = decls;
    return this;
}

Type *new_typedef_type(char *typedef_name)
{
    Type *this = new_type(TY_TYPEDEF, -1);
    this->typedef_name = typedef_name;
    return this;
}

Type *new_enum_type(char *name, Vector *list)
{
    Type *this = new_type(TY_ENUM, -1);
    this->enname = name;
    this->enum_list = list;
    return this;
}

Type *new_static_type(Type *type)
{
    Type *this = safe_malloc(sizeof(Type));
    memcpy(this, type, sizeof(Type));
    this->is_static = 1;
    return this;
}
