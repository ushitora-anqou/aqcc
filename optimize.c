#include "aqcc.h"

AST *optimize_ast_constant_detail(AST *ast)
{
    if (ast == NULL) return ast;

    switch (ast->kind) {
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        case AST_REM:
        case AST_LSHIFT:
        case AST_RSHIFT:
        case AST_LT:
        case AST_LTE:
        case AST_EQ:
        case AST_AND:
        case AST_XOR:
        case AST_OR:
        case AST_LAND:
        case AST_LOR:
        case AST_NEQ:
        case AST_ASSIGN:
        case AST_LVAR_DECL_INIT:
        case AST_GVAR_DECL_INIT:
            ast->lhs = optimize_ast_constant_detail(ast->lhs);
            ast->rhs = optimize_ast_constant_detail(ast->rhs);
            return ast;

        case AST_COMPL:
        case AST_UNARY_MINUS:
        case AST_NOT:
        case AST_EXPR_STMT:
        case AST_RETURN:
        case AST_PREINC:
        case AST_POSTINC:
        case AST_PREDEC:
        case AST_POSTDEC:
        case AST_ADDR:
        case AST_INDIR:
        case AST_CAST:
        case AST_LVALUE2RVALUE:
            ast->lhs = optimize_ast_constant_detail(ast->lhs);
            return ast;

        case AST_ARY2PTR:
            ast->ary = optimize_ast_constant_detail(ast->ary);
            return ast;

        case AST_COND:
            ast->cond = optimize_ast_constant_detail(ast->cond);
            ast->then = optimize_ast_constant_detail(ast->then);
            ast->els = optimize_ast_constant_detail(ast->els);
            return ast;

        case AST_EXPR_LIST:
            for (int i = 0; i < vector_size(ast->exprs); i++)
                vector_set(ast->exprs, i,
                           optimize_ast_constant_detail(
                               (AST *)vector_get(ast->exprs, i)));
            return ast;

        case AST_DECL_LIST:
            for (int i = 0; i < vector_size(ast->decls); i++)
                vector_set(ast->decls, i,
                           optimize_ast_constant_detail(
                               (AST *)vector_get(ast->decls, i)));
            return ast;

        case AST_FUNCCALL:
            for (int i = 0; i < vector_size(ast->args); i++)
                vector_set(ast->args, i,
                           optimize_ast_constant_detail(
                               (AST *)vector_get(ast->args, i)));
            return ast;

        case AST_FUNCDEF:
            ast->body = optimize_ast_constant_detail(ast->body);
            return ast;

        case AST_COMPOUND:
            for (int i = 0; i < vector_size(ast->stmts); i++)
                vector_set(ast->stmts, i,
                           optimize_ast_constant_detail(
                               (AST *)vector_get(ast->stmts, i)));
            return ast;

        case AST_IF:
            ast->cond = optimize_ast_constant_detail(ast->cond);
            ast->then = optimize_ast_constant_detail(ast->then);
            ast->els = optimize_ast_constant_detail(ast->els);
            return ast;

        case AST_LABEL:
            ast->label_stmt = optimize_ast_constant_detail(ast->label_stmt);
            return ast;

        case AST_CASE:
            ast->lhs = optimize_ast_constant_detail(ast->lhs);
            ast->rhs = optimize_ast_constant_detail(ast->rhs);
            return ast;

        case AST_DEFAULT:
            ast->lhs = optimize_ast_constant_detail(ast->lhs);
            return ast;

        case AST_SWITCH:
            ast->target = optimize_ast_constant_detail(ast->target);
            ast->switch_body = optimize_ast_constant_detail(ast->switch_body);
            return ast;

        case AST_DOWHILE:
            ast->cond = optimize_ast_constant_detail(ast->cond);
            ast->then = optimize_ast_constant_detail(ast->then);
            return ast;

        case AST_FOR:
            ast->initer = optimize_ast_constant_detail(ast->initer);
            ast->midcond = optimize_ast_constant_detail(ast->midcond);
            ast->iterer = optimize_ast_constant_detail(ast->iterer);
            ast->for_body = optimize_ast_constant_detail(ast->for_body);
            return ast;

        case AST_MEMBER_REF:
            ast->stsrc = optimize_ast_constant_detail(ast->stsrc);
            return ast;
    }

    return ast;
}

void optimize_ast_constant(Vector *asts)
{
    for (int i = 0; i < vector_size(asts); i++)
        vector_set(asts, i,
                   optimize_ast_constant_detail((AST *)vector_get(asts, i)));
}
