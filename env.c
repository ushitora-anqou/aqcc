#include "aqcc.h"

Env *new_env(Env *parent)
{
    Env *this;

    this = safe_malloc(sizeof(Env));
    this->parent = parent;
    this->symbols = new_map();
    this->scoped_vars = parent == NULL ? new_vector() : parent->scoped_vars;
    this->types = new_map();
    this->enum_values = new_map();
    return this;
}

AST *add_symbol(Env *env, const char *name, AST *ast)
{
    KeyValue *kv;

    kv = map_lookup(env->symbols, name);
    if (kv != NULL) error("same symbol already exists: '%s'", name);
    map_insert(env->symbols, name, ast);
    return ast;
}

AST *lookup_symbol(Env *env, const char *name)
{
    KeyValue *kv = map_lookup(env->symbols, name);
    if (kv == NULL) {
        if (env->parent == NULL) return NULL;
        return lookup_symbol(env->parent, name);
    }

    return (AST *)kv_value(kv);
}

AST *add_var(Env *env, AST *ast)
{
    AST *var;

    assert(ast->kind == AST_LVAR_DECL || ast->kind == AST_GVAR_DECL);
    assert(ast->kind != AST_GVAR_DECL || env->parent == NULL);

    // Create a local/global variable instance.
    // All AST_VAR that point the same variable will be replaced with
    // the pointer to this AST_LVAR/AST_GVAR instance when analyzing.
    var = new_ast(ast->kind == AST_LVAR_DECL ? AST_LVAR : AST_GVAR);
    var->type = ast->type;
    var->varname = ast->varname;
    var->stack_idx = -1;

    add_symbol(env, ast->varname, var);
    vector_push_back(env->scoped_vars, var);

    return ast;
}

AST *lookup_var(Env *env, const char *name)
{
    AST *ast;

    ast = lookup_symbol(env, name);
    if (ast && ast->kind != AST_LVAR && ast->kind != AST_GVAR)
        error("found but not var");
    return ast;
}

AST *add_func(Env *env, const char *name, AST *ast)
{
    assert(ast->kind == AST_FUNCDEF || ast->kind == AST_FUNC_DECL);
    add_symbol(env, name, ast);

    return ast;
}

AST *lookup_func(Env *env, const char *name)
{
    AST *ast;

    ast = lookup_symbol(env, name);
    if (ast && ast->kind != AST_FUNCDEF && ast->kind != AST_FUNC_DECL)
        error("found but not func");
    return ast;
}

Type *add_type(Env *env, Type *type, char *name)
{
    KeyValue *kv = map_lookup(env->types, name);
    if (kv != NULL) error("same type already exists: '%s'", name);
    map_insert(env->types, name, type);
    return type;
}

Type *lookup_type(Env *env, const char *name)
{
    KeyValue *kv = map_lookup(env->types, name);
    if (kv == NULL) {
        if (env->parent == NULL) return NULL;
        return lookup_type(env->parent, name);
    }
    return (Type *)kv_value(kv);
}

Type *add_struct_or_union_or_enum_type(Env *env, Type *type)
{
    assert(type->kind == TY_STRUCT || type->kind == TY_UNION ||
           type->kind == TY_ENUM);
    return add_type(env, type, format("struct/union %s", type->stname));
}

Type *lookup_struct_or_union_or_enum_type(Env *env, const char *name)
{
    Type *type = lookup_type(env, format("struct/union %s", name));
    assert(type == NULL || type->kind == TY_STRUCT || type->kind == TY_UNION ||
           type->kind == TY_ENUM);
    return type;
}

int add_enum_value(Env *env, char *name, int ival)
{
    if (lookup_enum_value(env, name)) error("duplicate enum name: '%s'", name);
    map_insert(env->enum_values, name, new_int(ival));
    return ival;
}

int *lookup_enum_value(Env *env, char *name)
{
    KeyValue *kv = map_lookup(env->enum_values, name);
    if (kv == NULL) {
        if (env->parent == NULL) return NULL;
        return lookup_enum_value(env->parent, name);
    }
    return kv_value(kv);
}
