#include "aqcc.h"

Env *new_env(Env *parent)
{
    Env *this;

    this = safe_malloc(sizeof(Env));
    this->parent = parent;
    this->symbols = new_map();
    this->scoped_vars = parent == NULL ? NULL : parent->scoped_vars;
    return this;
}

AST *add_symbol(Env *env, const char *name, AST *ast)
{
    KeyValue *kv;

    kv = map_lookup(env->symbols, name);
    if (kv != NULL) error("same symbol already exists.", __FILE__, __LINE__);
    map_insert(env->symbols, name, ast);
    return ast;
}

AST *lookup_symbol(Env *env, const char *name)
{
    KeyValue *kv;

    kv = map_lookup(env->symbols, name);
    if (kv == NULL) {
        if (env->parent == NULL) return NULL;
        return lookup_symbol(env->parent, name);
    }

    return (AST *)kv_value(kv);
}

AST *add_var(Env *env, AST *ast)
{
    AST *var;

    assert(ast->kind == AST_LVAR_DECL);

    // Create a local variable instance.
    // All AST_LVAR pointers that have `ast->varname`  will swap the pointer to
    // this instance when analysis.
    var = new_ast(AST_LVAR);
    var->type = ast->type;
    var->varname = ast->varname;
    var->stack_idx = -1;  // dummy

    add_symbol(env, ast->varname, var);
    vector_push_back(env->scoped_vars, var);

    return ast;
}

AST *lookup_var(Env *env, const char *name)
{
    AST *ast;

    ast = lookup_symbol(env, name);
    if (ast && ast->kind != AST_LVAR)
        error("found but not var", __FILE__, __LINE__);
    return ast;
}

AST *add_func(Env *env, const char *name, AST *ast)
{
    assert(ast->kind == AST_FUNCDEF || ast->kind == AST_FUNCDECL);
    add_symbol(env, name, ast);

    return ast;
}

AST *lookup_func(Env *env, const char *name)
{
    AST *ast;

    ast = lookup_symbol(env, name);
    if (ast && ast->kind != AST_FUNCDEF && ast->kind != AST_FUNCDECL)
        error("found but not func", __FILE__, __LINE__);
    return ast;
}
