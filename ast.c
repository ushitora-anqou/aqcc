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

AST *new_binop_ast(int kind, AST *lhs, AST *rhs, Type *type)
{
    AST *ast = new_ast(kind);

    ast->type = type;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

AST *new_funccall_ast(char *fname, Vector *args, Type *ret_type)
{
    AST *ast = new_ast(AST_FUNCCALL);
    ast->fname = fname;
    ast->args = args;
    ast->type = ret_type;
    return ast;
}

AST *new_funcdef_ast(char *fname, Vector *params, Type *ret_type)
{
    AST *ast = new_ast(AST_FUNCDEF);
    ast->fname = fname;
    ast->params = params;
    ast->body = NULL;
    ast->env = NULL;
    ast->ret_type = ret_type;
    return ast;
}

AST *new_expr_stmt(AST *expr)
{
    AST *ast;

    ast = new_ast(AST_EXPR_STMT);
    ast->lhs = expr;
    ast->rhs = NULL;  // not used
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

AST *new_indir_ast(AST *src)
{
    AST *ast;

    if (!match_type(src, TY_PTR))
        error("pointer should come after indirection op", __FILE__, __LINE__);
    ast = new_ast(AST_INDIR);
    ast->lhs = src;
    ast->type = src->type->ptr_of;
    return ast;
}

AST *ary2ptr(AST *ary)
{
    if (!match_type(ary, TY_ARY)) return ary;
    return new_ary2ptr_ast(ary);
}

AST *new_add_ast(AST *lhs, AST *rhs)
{
    Type *ret_type;

    lhs = ary2ptr(lhs);
    rhs = ary2ptr(rhs);

    if (match_type2(lhs, rhs, TY_PTR, TY_PTR))
        error("ptr + ptr is not allowed", __FILE__, __LINE__);

    // convert ptr+int to int+ptr
    if (match_type(lhs, TY_PTR)) {
        // swap(lhs, rhs)
        AST *tmp;
        tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }

    ret_type = rhs->type;

    return new_binop_ast(AST_ADD, lhs, rhs, ret_type);
}
