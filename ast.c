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
    ast->type = ret_type;
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

AST *char2int(AST *ch)
{
    AST *ast;

    if (!match_type(ch, TY_CHAR)) return ch;
    ast = new_ast(AST_CHAR2INT);
    ast->lhs = ch;
    ast->type = type_int();
    return ast;
}

AST *new_var_ast(char *varname)
{
    AST *ast;

    ast = new_ast(AST_VAR);
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

AST *new_var_decl_ast(int kind, Type *type, char *varname)
{
    AST *ast;

    ast = new_ast(kind);
    ast->type = type;
    ast->varname = varname;
    return ast;
}

AST *new_var_decl_init_ast(AST *var_decl, AST *initer)
{
    if (var_decl->kind != AST_LVAR_DECL && var_decl->kind != AST_GVAR_DECL)
        error("only local/global variable can have initializer.");

    AST *ast = new_ast(var_decl->kind == AST_LVAR_DECL ? AST_LVAR_DECL_INIT
                                                       : AST_GVAR_DECL_INIT);
    ast->lhs = var_decl;
    ast->rhs =
        new_binop_ast(AST_ASSIGN, new_var_ast(var_decl->varname), initer);
    return ast;
}

AST *new_label_ast(char *name, AST *stmt)
{
    AST *label = new_ast(AST_LABEL);
    label->label_name = name;
    label->label_stmt = stmt;
    return label;
}

AST *new_lvalue2rvalue_ast(AST *lvalue)
{
    AST *this = new_ast(AST_LVALUE2RVALUE);
    this->lhs = lvalue;
    this->type = lvalue->type;
    return this;
}

AST *new_int_ast(int ival)
{
    AST *this = new_ast(AST_INT);
    this->ival = ival;
    this->type = type_int();
    return this;
}
