#include "aqcc.h"

AST *parse_expr(Env *env, TokenSeq *tokseq);
AST *parse_assignment_expr(Env *env, TokenSeq *tokseq);
AST *parse_stmt(Env *env, TokenSeq *tokseq);

TokenSeq *new_token_seq(Vector *tokens)
{
    TokenSeq *this = safe_malloc(sizeof(TokenSeq));

    this->tokens = tokens;
    this->idx = 0;
    return this;
}

Token *peek_token(TokenSeq *seq)
{
    Token *token = vector_get(seq->tokens, seq->idx);
    if (token == NULL) error("no next token.", __FILE__, __LINE__);
    return token;
}

Token *pop_token(TokenSeq *seq)
{
    Token *token = vector_get(seq->tokens, seq->idx++);
    if (token == NULL) error("no next token.", __FILE__, __LINE__);
    return token;
}

Token *expect_token(TokenSeq *seq, int kind)
{
    Token *token = pop_token(seq);
    if (token->kind != kind) error("unexpected token.", __FILE__, __LINE__);
    return token;
}

int match_token(TokenSeq *seq, int kind)
{
    Token *token = peek_token(seq);
    return token->kind == kind;
}

int match_token2(TokenSeq *seq, int kind0, int kind1)
{
    Token *token = peek_token(seq);
    if (token->kind != kind0) return 0;
    seq->idx++;
    token = peek_token(seq);
    seq->idx--;
    if (token->kind != kind1) return 0;
    return 1;
}

TokenSeqSaved *new_token_seq_saved(TokenSeq *tokseq)
{
    TokenSeqSaved *this;

    this = (TokenSeqSaved *)safe_malloc(sizeof(TokenSeqSaved));
    this->idx = tokseq->idx;

    return this;
}

void restore_token_seq_saved(TokenSeqSaved *saved, TokenSeq *tokseq)
{
    tokseq->idx = saved->idx;
}

#define SAVE_TOKSEQ \
    TokenSeqSaved *token_seq_saved__dummy = new_token_seq_saved(tokseq);
#define RESTORE_TOKSEQ restore_token_seq_saved(token_seq_saved__dummy, tokseq);

AST *parse_primary_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast;
    Token *token = pop_token(tokseq);
    switch (token->kind) {
        case tINT:
            ast = new_ast(AST_INT);
            ast->ival = token->ival;
            ast->type = type_int();
            break;

        case tLPAREN:
            ast = parse_expr(env, tokseq);
            expect_token(tokseq, tRPAREN);
            break;

        case tIDENT:
            // ALL tIDENT should be handled in parse_postfix_expr()
            assert(0);

        default:
            error("unexpected token", __FILE__, __LINE__);
    }
    return ast;
}

Vector *parse_argument_expr_list(Env *env, TokenSeq *tokseq)
{
    Vector *args = new_vector();

    if (match_token(tokseq, tRPAREN)) return args;

    while (1) {
        vector_push_back(args, parse_assignment_expr(env, tokseq));
        if (match_token(tokseq, tRPAREN)) break;
        expect_token(tokseq, tCOMMA);
    }

    return args;
}

AST *parse_postfix_expr(Env *env, TokenSeq *tokseq)
{
    // TODO: should be recursive

    if (match_token2(tokseq, tIDENT, tLPAREN)) {  // function call
        Token *ident;
        AST *ast;
        Type *type;

        ident = pop_token(tokseq);
        pop_token(tokseq);

        ast = lookup_func(env, ident->sval);
        if (ast == NULL) {
            warn("function call type is deduced to int", __FILE__, __LINE__);
            warn(ident->sval, __FILE__, __LINE__);
            type = type_int();
        }
        else {
            type = ast->ret_type;
        }

        ast = new_funccall_ast(ident->sval,
                               parse_argument_expr_list(env, tokseq), type);
        expect_token(tokseq, tRPAREN);
        return ast;
    }

    AST *ast;

    if (match_token(tokseq, tIDENT)) {  // variable
        Token *ident;

        ident = pop_token(tokseq);
        ast = lookup_var(env, ident->sval);
        if (ast == NULL) error("not declared variable", __FILE__, __LINE__);
    }
    else {
        ast = parse_primary_expr(env, tokseq);
    }

    while (1) {
        switch (peek_token(tokseq)->kind) {
            case tINC: {
                AST *inc;

                pop_token(tokseq);
                if (ast->kind != AST_LVAR)
                    error("not variable name", __FILE__, __LINE__);
                inc = new_ast(AST_POSTINC);
                inc->type = ast->type;
                inc->lhs = ast;
                ast = inc;
            } break;

            case tLBRACKET:
                pop_token(tokseq);
                ast = new_indir_ast(new_add_ast(parse_expr(env, tokseq), ast));
                expect_token(tokseq, tRBRACKET);
                break;

            default:
                goto end;
        }
    }

end:
    return ast;
}

AST *parse_unary_expr(Env *env, TokenSeq *tokseq)
{
    Token *token = peek_token(tokseq);
    switch (token->kind) {
        case tPLUS:
            pop_token(tokseq);
            return parse_postfix_expr(env, tokseq);

        case tMINUS: {
            AST *ast;

            pop_token(tokseq);
            ast = new_ast(AST_UNARY_MINUS);
            ast->lhs = parse_unary_expr(env, tokseq);
            ast->type = ast->lhs->type;
            return ast;
        }

        case tINC: {
            AST *ast, *inc;

            pop_token(tokseq);
            ast = parse_unary_expr(env, tokseq);
            if (ast->kind != AST_LVAR)
                error("unexpected token", __FILE__, __LINE__);
            inc = new_ast(AST_PREINC);
            inc->type = ast->type;
            inc->lhs = ast;
            return inc;
        }

        case tAND: {
            AST *ast;

            pop_token(tokseq);
            ast = new_ast(AST_ADDR);
            ast->lhs = parse_unary_expr(env, tokseq);
            if (ast->lhs->kind != AST_LVAR)
                error("var should be here.", __FILE__, __LINE__);
            ast->type = new_pointer_type(ast->lhs->type);
            return ast;
        }

        case tSTAR:
            pop_token(tokseq);
            return new_indir_ast(parse_unary_expr(env, tokseq));
    }

    return parse_postfix_expr(env, tokseq);
}

AST *parse_multiplicative_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_unary_expr(env, tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tSTAR:
                pop_token(tokseq);
                ast = new_binop_ast(AST_MUL, ast, parse_unary_expr(env, tokseq),
                                    ast->type);
                break;
            case tSLASH:
                pop_token(tokseq);
                ast = new_binop_ast(AST_DIV, ast, parse_unary_expr(env, tokseq),
                                    ast->type);
                break;
            case tPERCENT:
                pop_token(tokseq);
                ast = new_binop_ast(AST_REM, ast, parse_unary_expr(env, tokseq),
                                    ast->type);
                break;
            default:
                return ast;
        }
    }
}

AST *parse_additive_expr(Env *env, TokenSeq *tokseq)
{
    AST *lhs, *rhs;

    lhs = parse_multiplicative_expr(env, tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tPLUS:

                pop_token(tokseq);
                rhs = parse_multiplicative_expr(env, tokseq);
                lhs = new_add_ast(lhs, rhs);
                break;

            case tMINUS: {
                Type *ret_type;

                pop_token(tokseq);
                rhs = parse_multiplicative_expr(env, tokseq);

                lhs = ary2ptr(lhs);
                rhs = ary2ptr(rhs);

                if (match_type2(lhs, rhs, TY_INT, TY_PTR))
                    error("int - ptr is not allowed", __FILE__, __LINE__);

                ret_type = lhs->type;
                if (match_type2(lhs, rhs, TY_PTR, TY_PTR))
                    ret_type = type_int();  // TODO: long

                lhs = new_binop_ast(AST_SUB, lhs, rhs, ret_type);
            } break;

            default:
                return lhs;
        }
    }
}

AST *parse_shift_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_additive_expr(env, tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tLSHIFT:
                pop_token(tokseq);
                ast =
                    new_binop_ast(AST_LSHIFT, ast,
                                  parse_additive_expr(env, tokseq), ast->type);
                break;
            case tRSHIFT:
                pop_token(tokseq);
                ast =
                    new_binop_ast(AST_RSHIFT, ast,
                                  parse_additive_expr(env, tokseq), ast->type);
                break;
            default:
                return ast;
        }
    }
}

AST *parse_relational_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_shift_expr(env, tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tLT:
                pop_token(tokseq);
                ast = new_binop_ast(AST_LT, ast, parse_shift_expr(env, tokseq),
                                    ast->type);
                break;
            case tGT:
                pop_token(tokseq);
                ast = new_binop_ast(AST_GT, ast, parse_shift_expr(env, tokseq),
                                    ast->type);
                break;
            case tLTE:
                pop_token(tokseq);
                ast = new_binop_ast(AST_LTE, ast, parse_shift_expr(env, tokseq),
                                    ast->type);
                break;
            case tGTE:
                pop_token(tokseq);
                ast = new_binop_ast(AST_GTE, ast, parse_shift_expr(env, tokseq),
                                    ast->type);
                break;
            default:
                return ast;
        }
    }
}

AST *parse_equality_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_relational_expr(env, tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tEQEQ:
                pop_token(tokseq);
                ast = new_binop_ast(
                    AST_EQ, ast, parse_relational_expr(env, tokseq), ast->type);
                break;
            case tNEQ:
                pop_token(tokseq);
                ast = new_binop_ast(AST_NEQ, ast,
                                    parse_relational_expr(env, tokseq),
                                    ast->type);
                break;
            default:
                return ast;
        }
    }
}

AST *parse_and_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_equality_expr(env, tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tAND) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_AND, ast, parse_equality_expr(env, tokseq),
                            ast->type);
    }
}

AST *parse_exclusive_or_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_and_expr(env, tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tHAT) return ast;
        pop_token(tokseq);
        ast =
            new_binop_ast(AST_XOR, ast, parse_and_expr(env, tokseq), ast->type);
    }
}

AST *parse_inclusive_or_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_exclusive_or_expr(env, tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tBAR) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_OR, ast, parse_exclusive_or_expr(env, tokseq),
                            ast->type);
    }
}

AST *parse_logical_and_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_inclusive_or_expr(env, tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tANDAND) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_LAND, ast, parse_inclusive_or_expr(env, tokseq),
                            ast->type);
    }
}

AST *parse_logical_or_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_logical_and_expr(env, tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tBARBAR) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_LOR, ast, parse_logical_and_expr(env, tokseq),
                            ast->type);
    }
}

AST *parse_conditional_expr(Env *env, TokenSeq *tokseq)
{
    AST *cond, *then, *els, *ast;

    cond = parse_logical_or_expr(env, tokseq);
    if (!match_token(tokseq, tQUESTION)) return cond;
    pop_token(tokseq);
    then = parse_expr(env, tokseq);
    expect_token(tokseq, tCOLON);
    els = parse_conditional_expr(env, tokseq);

    ast = new_ast(AST_COND);
    ast->type = then->type;  // TODO: should be determined by both of then
                             // and els's types.
    ast->cond = cond;
    ast->then = then;
    ast->els = els;
    return ast;
}

AST *parse_assignment_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast;

    SAVE_TOKSEQ;
    ast = parse_unary_expr(env, tokseq);
    if (!match_token(tokseq, tEQ)) {
        RESTORE_TOKSEQ;
        return parse_conditional_expr(env, tokseq);
    }
    pop_token(tokseq);

    if (ast->kind == AST_INDIR)
        return new_binop_ast(AST_ASSIGN, ast,
                             parse_assignment_expr(env, tokseq), ast->type);

    if (ast->kind != AST_LVAR)
        error("only lvalue can be lhs of assignment.", __FILE__, __LINE__);

    return new_binop_ast(AST_ASSIGN, ast, parse_assignment_expr(env, tokseq),
                         ast->type);
}

AST *parse_expr(Env *env, TokenSeq *tokseq)
{
    return parse_assignment_expr(env, tokseq);
}

AST *parse_expression_stmt(Env *env, TokenSeq *tokseq)
{
    AST *expr = NULL;

    if (!match_token(tokseq, tSEMICOLON)) expr = parse_expr(env, tokseq);
    expect_token(tokseq, tSEMICOLON);

    return new_expr_stmt(expr);
}

AST *parse_jump_stmt(Env *env, TokenSeq *tokseq)
{
    Token *token = pop_token(tokseq);
    AST *ast = NULL;

    switch (token->kind) {
        case kRETURN:
            if (!match_token(tokseq, tSEMICOLON)) ast = parse_expr(env, tokseq);
            ast = new_binop_ast(AST_RETURN, ast, NULL, NULL);
            expect_token(tokseq, tSEMICOLON);
            break;
        case kBREAK:
            ast = new_ast(AST_BREAK);
            expect_token(tokseq, tSEMICOLON);
            break;
        case kCONTINUE:
            ast = new_ast(AST_CONTINUE);
            expect_token(tokseq, tSEMICOLON);
            break;
        default:
            error("unexpected token", __FILE__, __LINE__);
    }

    return ast;
}

AST *parse_iteration_stmt(Env *env, TokenSeq *tokseq)
{
    Token *token = pop_token(tokseq);
    AST *ast;

    switch (token->kind) {
        case kWHILE: {
            AST *cond, *body;
            expect_token(tokseq, tLPAREN);
            cond = parse_expr(env, tokseq);
            expect_token(tokseq, tRPAREN);
            body = parse_stmt(env, tokseq);

            ast = new_while_stmt(cond, body);
        } break;

        case kFOR: {
            AST *initer, *cond, *iterer, *body;

            expect_token(tokseq, tLPAREN);
            if (match_token(tokseq, tSEMICOLON))
                initer = NULL;
            else
                initer = parse_expr(env, tokseq);
            expect_token(tokseq, tSEMICOLON);
            if (match_token(tokseq, tSEMICOLON))
                cond = NULL;
            else
                cond = parse_expr(env, tokseq);
            expect_token(tokseq, tSEMICOLON);
            if (match_token(tokseq, tRPAREN))
                iterer = NULL;
            else
                iterer = parse_expr(env, tokseq);
            expect_token(tokseq, tRPAREN);
            body = parse_stmt(env, tokseq);

            ast = new_ast(AST_FOR);
            ast->initer = initer;
            ast->midcond = cond;
            ast->iterer = iterer;
            ast->for_body = body;
        } break;

        default:
            error("unexpected token", __FILE__, __LINE__);
    }

    return ast;
}

Type *parse_type_name(Env *env, TokenSeq *tokseq)
{
    Type *type;

    expect_token(tokseq, kINT);
    type = type_int();

    while (match_token(tokseq, tSTAR)) {
        pop_token(tokseq);
        type = new_pointer_type(type);
    }

    return type;
}

void parse_declaration(Env *env, TokenSeq *tokseq)
{
    AST *ast;

    ast = new_ast(AST_LVAR);
    ast->type = parse_type_name(env, tokseq);
    ast->varname = expect_token(tokseq, tIDENT)->sval;

    while (match_token(tokseq, tLBRACKET)) {  // array
        Token *num;

        pop_token(tokseq);
        num = expect_token(tokseq, tINT);  // TODO: parse assignment-expr
        expect_token(tokseq, tRBRACKET);
        ast->type = new_array_type(ast->type, num->ival);
    }

    expect_token(tokseq, tSEMICOLON);

    add_var(env, ast->varname, ast);
}

AST *parse_compound_stmt(Env *env, TokenSeq *tokseq)
{
    AST *ast;
    Vector *stmts = new_vector();
    Env *nenv = new_env(env);

    expect_token(tokseq, tLBRACE);
    while (!match_token(tokseq, tRBRACE)) {
        if (match_token(tokseq, kINT))
            parse_declaration(nenv, tokseq);
        else
            vector_push_back(stmts, parse_stmt(nenv, tokseq));
    }
    expect_token(tokseq, tRBRACE);

    ast = new_ast(AST_COMPOUND);
    ast->stmts = stmts;

    return ast;
}

AST *parse_selection_stmt(Env *env, TokenSeq *tokseq)
{
    AST *ast, *cond, *then, *els = NULL;

    expect_token(tokseq, kIF);
    expect_token(tokseq, tLPAREN);
    cond = parse_expr(env, tokseq);
    expect_token(tokseq, tRPAREN);
    then = parse_stmt(env, tokseq);
    if (match_token(tokseq, kELSE)) {
        pop_token(tokseq);
        els = parse_stmt(env, tokseq);
    }

    ast = new_ast(AST_IF);
    ast->cond = cond;
    ast->then = then;
    ast->els = els;
    return ast;
}

AST *parse_stmt(Env *env, TokenSeq *tokseq)
{
    Token *token = peek_token(tokseq);

    switch (token->kind) {
        case kBREAK:
        case kCONTINUE:
        case kRETURN:
            return parse_jump_stmt(env, tokseq);
        case tLBRACE:
            return parse_compound_stmt(env, tokseq);
        case kIF:
            return parse_selection_stmt(env, tokseq);
        case kWHILE:
        case kFOR:
            return parse_iteration_stmt(env, tokseq);
    }

    return parse_expression_stmt(env, tokseq);
}

Vector *parse_parameter_list(Env *env, TokenSeq *tokseq)
{
    Vector *params = new_vector();

    if (match_token(tokseq, tRPAREN)) return params;

    while (1) {
        Type *type = parse_type_name(env, tokseq);
        char *name = expect_token(tokseq, tIDENT)->sval;

        vector_push_back(params, new_pair(type, name));
        if (match_token(tokseq, tRPAREN)) break;
        expect_token(tokseq, tCOMMA);
    }

    return params;
}

AST *parse_function_definition_or_declaration(Env *env, TokenSeq *tokseq)
{
    char *fname;
    Vector *params;
    AST *func;
    int i;
    Type *ret_type;

    ret_type = NULL;
    if (match_token(tokseq, kINT)) ret_type = parse_type_name(env, tokseq);
    if (ret_type == NULL) {
        // warn("returning type is deduced to int", __FILE__, __LINE__);
        ret_type = type_int();
    }

    fname = expect_token(tokseq, tIDENT)->sval;
    expect_token(tokseq, tLPAREN);
    params = parse_parameter_list(env, tokseq);
    expect_token(tokseq, tRPAREN);

    // if semicolon follows the function parameter list,
    // it's a function declaration.
    if (match_token(tokseq, tSEMICOLON)) {
        pop_token(tokseq);
        add_func(env, fname, new_funcdef_ast(fname, params, ret_type));
        return NULL;
    }

    // already declared
    // TODO: validate params
    func = lookup_func(env, fname);
    if (func == NULL) {
        // ...or new defined function.
        func = new_funcdef_ast(fname, params, ret_type);
        add_func(env, fname, func);
    }

    // make function's local scope/env
    func->env = new_env(env);
    func->env->scoped_vars = new_vector();
    // add param into functions's scope
    // in reversed order for code generation.
    for (i = vector_size(params) - 1; i >= 0; i--) {
        AST *var;
        Pair *param = (Pair *)vector_get(params, i);

        var = new_ast(AST_LVAR);
        var->type = (Type *)(param->first);
        var->varname = (char *)(param->second);
        add_var(func->env, var->varname, var);
    }

    func->body = parse_compound_stmt(func->env, tokseq);

    return func;
}

Vector *parse_prog(Vector *tokens)
{
    Vector *asts;
    Env *env;
    TokenSeq *tokseq;

    asts = new_vector();
    env = new_env(NULL);
    tokseq = new_token_seq(tokens);

    while (peek_token(tokseq)->kind != tEOF) {
        AST *ast;

        ast = parse_function_definition_or_declaration(env, tokseq);
        if (ast != NULL) vector_push_back(asts, ast);
    }

    return asts;
}
