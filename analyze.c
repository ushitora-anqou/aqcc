#include "aqcc.h"

void swap(AST **lhs, AST **rhs)
{
    AST *tmp;
    tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

int is_lvalue(AST *ast)
{
    if (ast == NULL) return 0;

    switch (ast->kind) {
        case AST_LVAR:
        case AST_GVAR:
        case AST_INDIR:
            return 1;
    }

    return 0;
}

// Returns an analyzed AST pointer that may be the same one of `ast` or not.
// Caller should replace `ast` with the returned AST pointer.
AST *analyze_ast_detail(Env *env, AST *ast)
{
    if (ast == NULL) return NULL;

    switch (ast->kind) {
        case AST_INT:
            ast->type = type_int();
            break;

        case AST_ADD:
            ast->lhs = ary2ptr(analyze_ast_detail(env, ast->lhs));
            ast->rhs = ary2ptr(analyze_ast_detail(env, ast->rhs));

            if (match_type2(ast->lhs, ast->rhs, TY_PTR, TY_PTR))
                error("ptr + ptr is not allowed", __FILE__, __LINE__);

            // Convert ptr+int to int+ptr for easy code generation.
            if (match_type(ast->lhs, TY_PTR)) swap(&ast->lhs, &ast->rhs);

            ast->type = ast->rhs->type;
            break;

        case AST_SUB:
            ast->lhs = ary2ptr(analyze_ast_detail(env, ast->lhs));
            ast->rhs = ary2ptr(analyze_ast_detail(env, ast->rhs));

            if (match_type2(ast->lhs, ast->rhs, TY_INT, TY_PTR))
                error("int - ptr is not allowed", __FILE__, __LINE__);

            if (match_type2(ast->lhs, ast->rhs, TY_PTR, TY_PTR))
                ast->type = type_int();  // TODO: long
            else
                ast->type = ast->lhs->type;
            break;

        case AST_MUL:
        case AST_DIV:
        case AST_REM:
        case AST_LSHIFT:
        case AST_RSHIFT:
        case AST_LT:
        case AST_GT:
        case AST_LTE:
        case AST_GTE:
        case AST_EQ:
        case AST_NEQ:
        case AST_AND:
        case AST_XOR:
        case AST_OR:
        case AST_LAND:
        case AST_LOR:
            // TODO: ensure both lhs and rhs have arithmetic types or pointer
            // types if needed.
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            ast->rhs = analyze_ast_detail(env, ast->rhs);
            ast->type = ast->lhs->type;  // TODO: consider both lhs and rhs
            break;

        case AST_UNARY_MINUS:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            ast->type = ast->lhs->type;
            break;

        case AST_COND:
            ast->cond = analyze_ast_detail(env, ast->cond);
            ast->then = analyze_ast_detail(env, ast->then);
            ast->els = analyze_ast_detail(env, ast->els);
            ast->type = ast->then->type;  // TODO: consider both then and els
            break;

        case AST_ASSIGN:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            ast->rhs = analyze_ast_detail(env, ast->rhs);
            if (!is_lvalue(ast->lhs))
                error("should specify lvalue for assignment op", __FILE__,
                      __LINE__);
            ast->type = ast->lhs->type;
            break;

        case AST_VAR:
            ast = lookup_var(env, ast->varname);
            assert(ast->kind == AST_LVAR || ast->kind == AST_GVAR);
            if (!ast) error("not declared variable", __FILE__, __LINE__);
            break;

        case AST_LVAR_DECL:
        case AST_GVAR_DECL:
            // ast->type means this variable's type and is alraedy
            // filled when parsing.
            add_var(env, ast);
            break;

        case AST_FUNCCALL: {
            AST *funcdef;
            int i;

            funcdef = lookup_func(env, ast->fname);
            if (funcdef) {  // found: already declared
                ast->type = funcdef->ret_type;
            }
            else {
                warn("function call type is deduced to int", __FILE__,
                     __LINE__);
                warn(ast->fname, __FILE__, __LINE__);
                ast->type = type_int();
            }

            // analyze args
            for (i = 0; i < vector_size(ast->args); i++)
                vector_set(
                    ast->args, i,
                    analyze_ast_detail(env, (AST *)vector_get(ast->args, i)));
        } break;

        case AST_FUNC_DECL:
            add_func(env, ast->fname, ast);
            break;

        case AST_FUNCDEF: {
            AST *func;
            int i;

            func = lookup_func(env, ast->fname);
            if (func) {                  // already declared or defined.
                if (func->body != NULL)  // TODO: validate params
                    error(
                        "A function that has the same name as the defined one "
                        "already exists.",
                        __FILE__, __LINE__);
                func->body = ast->body;
            }
            else {  // no declaration
                add_func(env, ast->fname, ast);
            }

            // make function's local scope/env
            ast->env = new_env(env);
            ast->env->scoped_vars = new_vector();
            // add param into functions's scope
            // in reversed order for easy code generation.
            for (i = vector_size(ast->params) - 1; i >= 0; i--)
                add_var(ast->env, (AST *)vector_get(ast->params, i));

            // analyze body
            analyze_ast_detail(ast->env, ast->body);
        } break;

        case AST_EXPR_STMT:
        case AST_RETURN:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            break;

        case AST_COMPOUND: {
            int i;
            Env *nenv;

            nenv = new_env(env);
            for (i = 0; i < vector_size(ast->stmts); i++)
                analyze_ast_detail(nenv, (AST *)vector_get(ast->stmts, i));
        } break;

        case AST_IF:
        case AST_WHILE:
            ast->cond = analyze_ast_detail(env, ast->cond);
            ast->then = analyze_ast_detail(env, ast->then);
            ast->els = analyze_ast_detail(env, ast->els);
            break;

        case AST_FOR:
            ast->initer = analyze_ast_detail(env, ast->initer);
            ast->midcond = analyze_ast_detail(env, ast->midcond);
            ast->iterer = analyze_ast_detail(env, ast->iterer);
            ast->for_body = analyze_ast_detail(env, ast->for_body);
            break;

        case AST_PREINC:
        case AST_POSTINC:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            if (!is_lvalue(ast->lhs))
                error("should specify lvalue for pre increment", __FILE__,
                      __LINE__);
            ast->type = ast->lhs->type;
            break;

        case AST_ADDR:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            if (!is_lvalue(ast->lhs))
                error("should specify lvalue for address-of op", __FILE__,
                      __LINE__);
            ast->type = new_pointer_type(ast->lhs->type);
            break;

        case AST_INDIR:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            if (!match_type(ast->lhs, TY_PTR))
                error("pointer should come after indirection op", __FILE__,
                      __LINE__);
            ast->type = ast->lhs->type->ptr_of;
            break;

        case AST_ARY2PTR:
            assert(0);  // only made in analysis.

        case AST_GVAR:
            break;
    }

    return ast;
}

void analyze_ast(Vector *asts)
{
    int i;
    Env *env;

    env = new_env(NULL);

    for (i = 0; i < vector_size(asts); i++)
        vector_set(asts, i,
                   analyze_ast_detail(env, (AST *)vector_get(asts, i)));
}
