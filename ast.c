#include "aqcc.h"

int match_type(AST *ast, int kind)
{
    return ast->type != NULL && ast->type->kind == kind;
}

int match_type2(AST *lhs, AST *rhs, int lkind, int rkind)
{
    return match_type(lhs, lkind) && match_type(rhs, rkind);
}

AST *new_ast(int kind)
{
    AST *this = safe_malloc(sizeof(AST));
    this->kind = kind;
    this->type = NULL;
    return this;
}

AST *new_binop_ast(int kind, AST *lhs, AST *rhs)
{
    AST *ast = new_ast(kind);

    ast->type = NULL;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

AST *new_func_ast(int kind, char *fname, Vector *args, Vector *params,
                  Type *ret_type)
{
    AST *ast = new_ast(kind);
    ast->fname = fname;
    ast->args = args;
    ast->params = params;
    ast->ret_type = ret_type;
    ast->body = NULL;
    ast->env = NULL;
    return ast;
}

AST *new_while_stmt(AST *cond, AST *body)
{
    AST *ast;

    ast = new_ast(AST_WHILE);
    ast->cond = cond;
    ast->then = body;
    ast->els = NULL;  // not used

    return ast;
}

AST *new_compound_stmt2(AST *first, AST *second)
{
    AST *ast;
    Vector *stmts = new_vector();

    vector_push_back(stmts, first);
    vector_push_back(stmts, second);
    ast = new_ast(AST_COMPOUND);
    ast->stmts = stmts;

    return ast;
}

AST *new_ary2ptr_ast(AST *ary)
{
    AST *ast;

    assert(match_type(ary, TY_ARY));

    ast = new_ast(AST_ARY2PTR);
    ast->ary = ary;
    ast->type = new_pointer_type(ary->type->ary_of);

    return ast;
}

AST *ary2ptr(AST *ary)
{
    if (!match_type(ary, TY_ARY)) return ary;
    return new_ary2ptr_ast(ary);
}

AST *new_lvar_ast(char *varname)
{
    AST *ast;

    ast = new_ast(AST_LVAR);
    ast->varname = varname;
    return ast;
}

AST *new_unary_ast(int kind, AST *that)
{
    AST *ast;

    ast = new_ast(kind);
    ast->lhs = that;
    return ast;
}

AST *new_lvar_decl_ast(Type *type, char *varname)
{
    AST *ast;

    ast = new_ast(AST_LVAR_DECL);
    ast->type = type;
    ast->varname = varname;
    return ast;
}
