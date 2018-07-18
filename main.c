#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aqcc.h"

int max(int a, int b) { return a > b ? a : b; }

Env *new_env(Env *parent)
{
    Env *this;

    this = safe_malloc(sizeof(Env));
    this->parent = parent;
    this->local_vars = new_map();
    this->scoped_vars = parent == NULL ? NULL : parent->scoped_vars;
    return this;
}

AST *add_var(Env *env, const char *name, AST *ast)
{
    KeyValue *kv = map_lookup(env->local_vars, name);
    if (kv != NULL)
        error("var already exists in same scope.", __FILE__, __LINE__);
    map_insert(env->local_vars, name, ast);
    vector_push_back(env->scoped_vars, ast);
    return ast;
}

AST *lookup_var(Env *env, const char *name)
{
    KeyValue *kv = map_lookup(env->local_vars, name);
    if (kv == NULL) {
        if (env->parent == NULL) return NULL;
        return lookup_var(env->parent, name);
    }
    return (AST *)(kv->value);
}

Type *new_type(int kind)
{
    Type *this;

    this = safe_malloc(sizeof(Type));
    this->kind = kind;
    return this;
}

Token *new_token(int kind)
{
    Token *token = safe_malloc(sizeof(Token));
    token->kind = kind;
    return token;
}

Token *read_next_int_token(FILE *fh)
{
    char buf[256];  // TODO: enough length?
    int bufidx = 0;

    while (1) {
        int ch = fgetc(fh);

        if (!isdigit(ch)) {
            ungetc(ch, fh);
            break;
        }

        buf[bufidx++] = ch;
    }

    Token *token = new_token(tINT);
    buf[bufidx++] = '\0';
    token->ival = atoi(buf);
    return token;
}

Token *read_next_ident_token(FILE *fh)
{
    char buf[256];  // TODO: enough length?
    int bufidx = 0;

    buf[bufidx++] = fgetc(fh);
    while (1) {
        int ch = fgetc(fh);

        if (!isalnum(ch) && ch != '_') {
            ungetc(ch, fh);
            break;
        }

        buf[bufidx++] = ch;
    }
    buf[bufidx++] = '\0';

    if (strcmp(buf, "return") == 0) return new_token(kRETURN);
    if (strcmp(buf, "if") == 0) return new_token(kIF);
    if (strcmp(buf, "else") == 0) return new_token(kELSE);
    if (strcmp(buf, "while") == 0) return new_token(kWHILE);
    if (strcmp(buf, "break") == 0) return new_token(kBREAK);
    if (strcmp(buf, "continue") == 0) return new_token(kCONTINUE);
    if (strcmp(buf, "for") == 0) return new_token(kFOR);
    if (strcmp(buf, "int") == 0) return new_token(kINT);

    Token *token = new_token(tIDENT);
    token->sval = new_str(buf);
    return token;
}

Token *read_next_token(FILE *fh)
{
    while (1) {
        int ch;

        ch = fgetc(fh);

        if (isspace(ch)) continue;

        if (isdigit(ch)) {
            ungetc(ch, fh);
            return read_next_int_token(fh);
        }

        if (isalpha(ch) || ch == '_') {
            ungetc(ch, fh);
            return read_next_ident_token(fh);
        }

        switch (ch) {
            case '+':
                ch = fgetc(fh);
                if (ch == '+') return new_token(tINC);
                ungetc(ch, fh);
                return new_token(tPLUS);
            case '-':
                return new_token(tMINUS);
            case '*':
                return new_token(tSTAR);
            case '/':
                return new_token(tSLASH);
            case '%':
                return new_token(tPERCENT);
            case '(':
                return new_token(tLPAREN);
            case ')':
                return new_token(tRPAREN);
            case '<':
                ch = fgetc(fh);
                switch (ch) {
                    case '<':
                        return new_token(tLSHIFT);
                    case '=':
                        return new_token(tLTE);
                }
                ungetc(ch, fh);
                return new_token(tLT);
            case '>':
                ch = fgetc(fh);
                switch (ch) {
                    case '>':
                        return new_token(tRSHIFT);
                    case '=':
                        return new_token(tGTE);
                }
                ungetc(ch, fh);
                return new_token(tGT);
            case '=':
                ch = fgetc(fh);
                if (ch == '=') return new_token(tEQEQ);
                ungetc(ch, fh);
                return new_token(tEQ);
            case '!':
                ch = fgetc(fh);
                if (ch != '=') break;
                return new_token(tNEQ);
            case '&':
                ch = fgetc(fh);
                if (ch == '&') return new_token(tANDAND);
                ungetc(ch, fh);
                return new_token(tAND);
            case '^':
                return new_token(tHAT);
            case '|':
                ch = fgetc(fh);
                if (ch == '|') return new_token(tBARBAR);
                ungetc(ch, fh);
                return new_token(tBAR);
            case ';':
                return new_token(tSEMICOLON);
            case ',':
                return new_token(tCOMMA);
            case '{':
                return new_token(tLBRACE);
            case '}':
                return new_token(tRBRACE);
            case ':':
                return new_token(tCOLON);
            case '?':
                return new_token(tQUESTION);
            case EOF:
                return new_token(tEOF);
        }

        error("unexpected token", __FILE__, __LINE__);
    }
}

Vector *read_all_tokens(FILE *fh)
{
    Vector *tokens = new_vector();

    while (1) {
        Token *token = read_next_token(fh);
        vector_push_back(tokens, token);
        if (token->kind == tEOF) break;
    }

    return tokens;
}

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

AST *new_ast(int kind)
{
    AST *this = safe_malloc(sizeof(AST));
    this->kind = kind;
    return this;
}

AST *new_binop_ast(int kind, AST *lhs, AST *rhs)
{
    AST *ast = new_ast(kind);
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

AST *new_funccall_ast(char *fname, Vector *args)
{
    AST *ast = new_ast(AST_FUNCCALL);
    ast->fname = fname;
    ast->args = args;
    return ast;
}

AST *new_funcdef_ast(char *fname, Vector *params, AST *body, Env *env)
{
    AST *ast = new_ast(AST_FUNCDEF);
    ast->fname = fname;
    ast->params = params;
    ast->body = body;
    ast->env = env;
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

AST *parse_assignment_expr(Env *env, TokenSeq *tokseq);

AST *parse_primary_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast;
    Token *token = pop_token(tokseq);
    switch (token->kind) {
        case tINT:
            ast = new_ast(AST_INT);
            ast->ival = token->ival;
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
    // TODO: recursive

    if (match_token2(tokseq, tIDENT, tLPAREN)) {  // function call
        Token *ident;
        AST *ast;

        ident = pop_token(tokseq);
        pop_token(tokseq);
        ast = new_funccall_ast(ident->sval,
                               parse_argument_expr_list(env, tokseq));
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

    switch (peek_token(tokseq)->kind) {
        case tINC: {
            AST *inc;

            pop_token(tokseq);
            if (ast->kind != AST_VAR)
                error("not variable name", __FILE__, __LINE__);
            inc = new_ast(AST_POSTINC);
            inc->lhs = ast;
            ast = inc;
        } break;
    }

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
            ast->lhs = parse_postfix_expr(env, tokseq);
            return ast;
        }

        case tINC: {
            AST *ast, *inc;

            pop_token(tokseq);
            ast = parse_unary_expr(env, tokseq);
            if (ast->kind != AST_VAR)
                error("unexpected token", __FILE__, __LINE__);
            inc = new_ast(AST_PREINC);
            inc->lhs = ast;
            return inc;
        }
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
                ast =
                    new_binop_ast(AST_MUL, ast, parse_unary_expr(env, tokseq));
                break;
            case tSLASH:
                pop_token(tokseq);
                ast =
                    new_binop_ast(AST_DIV, ast, parse_unary_expr(env, tokseq));
                break;
            case tPERCENT:
                pop_token(tokseq);
                ast =
                    new_binop_ast(AST_REM, ast, parse_unary_expr(env, tokseq));
                break;
            default:
                return ast;
        }
    }
}

AST *parse_additive_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_multiplicative_expr(env, tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tPLUS:
                pop_token(tokseq);
                ast = new_binop_ast(AST_ADD, ast,
                                    parse_multiplicative_expr(env, tokseq));
                break;
            case tMINUS:
                pop_token(tokseq);
                ast = new_binop_ast(AST_SUB, ast,
                                    parse_multiplicative_expr(env, tokseq));
                break;
            default:
                return ast;
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
                ast = new_binop_ast(AST_LSHIFT, ast,
                                    parse_additive_expr(env, tokseq));
                break;
            case tRSHIFT:
                pop_token(tokseq);
                ast = new_binop_ast(AST_RSHIFT, ast,
                                    parse_additive_expr(env, tokseq));
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
                ast = new_binop_ast(AST_LT, ast, parse_shift_expr(env, tokseq));
                break;
            case tGT:
                pop_token(tokseq);
                ast = new_binop_ast(AST_GT, ast, parse_shift_expr(env, tokseq));
                break;
            case tLTE:
                pop_token(tokseq);
                ast =
                    new_binop_ast(AST_LTE, ast, parse_shift_expr(env, tokseq));
                break;
            case tGTE:
                pop_token(tokseq);
                ast =
                    new_binop_ast(AST_GTE, ast, parse_shift_expr(env, tokseq));
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
                ast = new_binop_ast(AST_EQ, ast,
                                    parse_relational_expr(env, tokseq));
                break;
            case tNEQ:
                pop_token(tokseq);
                ast = new_binop_ast(AST_NEQ, ast,
                                    parse_relational_expr(env, tokseq));
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
        ast = new_binop_ast(AST_AND, ast, parse_equality_expr(env, tokseq));
    }
}

AST *parse_exclusive_or_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_and_expr(env, tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tHAT) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_XOR, ast, parse_and_expr(env, tokseq));
    }
}

AST *parse_inclusive_or_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_exclusive_or_expr(env, tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tBAR) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_OR, ast, parse_exclusive_or_expr(env, tokseq));
    }
}

AST *parse_logical_and_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_inclusive_or_expr(env, tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tANDAND) return ast;
        pop_token(tokseq);
        ast =
            new_binop_ast(AST_LAND, ast, parse_inclusive_or_expr(env, tokseq));
    }
}

AST *parse_logical_or_expr(Env *env, TokenSeq *tokseq)
{
    AST *ast = parse_logical_and_expr(env, tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tBARBAR) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_LOR, ast, parse_logical_and_expr(env, tokseq));
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
    ast->cond = cond;
    ast->then = then;
    ast->els = els;
    return ast;
}

AST *parse_assignment_expr(Env *env, TokenSeq *tokseq)
{
    Token *token;
    AST *ast;

    if (!match_token2(tokseq, tIDENT, tEQ))
        return parse_conditional_expr(env, tokseq);
    token = expect_token(tokseq, tIDENT);
    expect_token(tokseq, tEQ);

    ast = lookup_var(env, token->sval);
    if (ast == NULL) error("not found such variable", __FILE__, __LINE__);
    return new_binop_ast(AST_ASSIGN, ast, parse_assignment_expr(env, tokseq));
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
            ast = new_binop_ast(AST_RETURN, ast, NULL);
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

AST *parse_stmt(Env *env, TokenSeq *tokseq);

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

void parse_declaration(Env *env, TokenSeq *tokseq)
{
    AST *ast;

    expect_token(tokseq, kINT);
    ast = new_ast(AST_VAR);
    ast->varname = expect_token(tokseq, tIDENT)->sval;
    ast->type = new_type(TY_INT);
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
        expect_token(tokseq, kINT);
        vector_push_back(params, expect_token(tokseq, tIDENT)->sval);
        if (match_token(tokseq, tRPAREN)) break;
        expect_token(tokseq, tCOMMA);
    }

    return params;
}

AST *parse_function_definition(Env *env, TokenSeq *tokseq)
{
    char *fname;
    Vector *params;
    Env *nenv;
    AST *stmts;
    int i;

    if (match_token(tokseq, kINT)) {
        pop_token(tokseq);
    }
    fname = expect_token(tokseq, tIDENT)->sval;
    expect_token(tokseq, tLPAREN);
    params = parse_parameter_list(env, tokseq);
    expect_token(tokseq, tRPAREN);

    nenv = new_env(env);
    nenv->scoped_vars = new_vector();
    // add param into nenv
    // add in reversed order for code generation.
    for (i = vector_size(params) - 1; i >= 0; i--) {
        AST *ast;

        ast = new_ast(AST_VAR);
        ast->varname = (char *)(vector_get(params, i));
        add_var(nenv, ast->varname, ast);
    }

    stmts = parse_compound_stmt(nenv, tokseq);

    return new_funcdef_ast(fname, params, stmts, nenv);
}

Vector *parse_prog(Env *env, TokenSeq *tokseq)
{
    Vector *asts = new_vector();

    while (peek_token(tokseq)->kind != tEOF) {
        vector_push_back(asts, parse_function_definition(env, tokseq));
    }

    return asts;
}

typedef struct {
    int nlabel, loop_continue_label, loop_break_label;
    Vector *codes;
} CodeEnv;

CodeEnv *new_code_env()
{
    CodeEnv *this;

    this = (CodeEnv *)safe_malloc(sizeof(CodeEnv));
    this->nlabel = 0;
    this->loop_continue_label = this->loop_break_label = -1;
    this->codes = new_vector();
    return this;
}

Vector *swap_codes(CodeEnv *env, Vector *new_codes)
{
    Vector *ret = env->codes;
    env->codes = new_codes;
    return ret;
}

void appcode(Vector *codes, const char *src, ...)
{
    char buf[256], buf2[256];  // TODO: enoguth length?
    int i, bufidx;
    va_list args;

    // copy src to buf.
    // replace # to %% in src.
    for (i = 0, bufidx = 0;; i++) {
        if (src[i] == '\0') break;

        if (src[i] == '#') {
            buf[bufidx++] = '%';
            buf[bufidx++] = '%';
            continue;
        }

        buf[bufidx++] = src[i];
    }
    buf[bufidx] = '\0';

    va_start(args, src);
    vsprintf(buf2, buf, args);
    va_end(args);

    vector_push_back(codes, new_str(buf2));
}

void dump_codes(Vector *codes, FILE *fh)
{
    int i;

    for (i = 0; i < vector_size(codes); i++)
        fprintf(fh, "%s\n", (const char *)vector_get(codes, i));
}

void generate_code(CodeEnv *env, Vector *asts);

void generate_code_detail(CodeEnv *env, AST *ast)
{
    const char *rreg[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    const char *ereg[] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};

    switch (ast->kind) {
        case AST_ADD:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "add #edi, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_SUB:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "sub #edi, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_MUL:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "imul #edi, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_DIV:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cltd");
            appcode(env->codes, "idiv #edi");
            appcode(env->codes, "push #rax");
            break;

        case AST_REM:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cltd");
            appcode(env->codes, "idiv #edi");
            appcode(env->codes, "push #rdx");
            break;

        case AST_UNARY_MINUS:
            generate_code_detail(env, ast->lhs);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "neg #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_LSHIFT:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rcx");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "sal #cl, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_RSHIFT:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rcx");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "sar #cl, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_LT:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp #edi, #eax");
            appcode(env->codes, "setl #al");
            appcode(env->codes, "movzb #al, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_GT:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp #edi, #eax");
            appcode(env->codes, "setg #al");
            appcode(env->codes, "movzb #al, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_LTE:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp #edi, #eax");
            appcode(env->codes, "setle #al");
            appcode(env->codes, "movzb #al, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_GTE:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp #edi, #eax");
            appcode(env->codes, "setge #al");
            appcode(env->codes, "movzb #al, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_EQ:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp #edi, #eax");
            appcode(env->codes, "sete #al");
            appcode(env->codes, "movzb #al, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_NEQ:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp #edi, #eax");
            appcode(env->codes, "setne #al");
            appcode(env->codes, "movzb #al, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_AND:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "and #edi, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_XOR:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "xor #edi, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_OR:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "or #edi, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_LAND: {
            int false_label = env->nlabel++, exit_label = env->nlabel++;
            generate_code_detail(env, ast->lhs);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp $0, #eax");
            appcode(env->codes, "je .L%d", false_label);
            // don't execute rhs expression if lhs is false.
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp $0, #eax");
            appcode(env->codes, "je .L%d", false_label);
            appcode(env->codes, "mov $1, #eax");
            appcode(env->codes, "jmp .L%d", exit_label);
            appcode(env->codes, ".L%d:", false_label);
            appcode(env->codes, "mov $0, #eax");
            appcode(env->codes, ".L%d:", exit_label);
            appcode(env->codes, "push #rax");
        } break;

        case AST_LOR: {
            int true_label = env->nlabel++, exit_label = env->nlabel++;
            generate_code_detail(env, ast->lhs);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp $0, #eax");
            appcode(env->codes, "jne .L%d", true_label);
            // don't execute rhs expression if lhs is true.
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp $0, #eax");
            appcode(env->codes, "jne .L%d", true_label);
            appcode(env->codes, "mov $0, #eax");
            appcode(env->codes, "jmp .L%d", exit_label);
            appcode(env->codes, ".L%d:", true_label);
            appcode(env->codes, "mov $1, #eax");
            appcode(env->codes, ".L%d:", exit_label);
            appcode(env->codes, "push #rax");
        } break;

        case AST_POSTINC: {
            appcode(env->codes, "push %d(#rbp)", ast->lhs->stack_idx);
            appcode(env->codes, "incl %d(#rbp)", ast->lhs->stack_idx);
        } break;

        case AST_PREINC: {
            appcode(env->codes, "incl %d(#rbp)", ast->lhs->stack_idx);
            appcode(env->codes, "push %d(#rbp)", ast->lhs->stack_idx);
        } break;

        case AST_IF:
        case AST_COND: {
            int false_label = env->nlabel++, exit_label = env->nlabel++;

            generate_code_detail(env, ast->cond);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp $0, #eax");
            appcode(env->codes, "je .L%d", false_label);
            generate_code_detail(env, ast->then);
            appcode(env->codes, "jmp .L%d", exit_label);
            appcode(env->codes, ".L%d:", false_label);
            if (ast->els != NULL) generate_code_detail(env, ast->els);
            appcode(env->codes, ".L%d:", exit_label);
        } break;

        case AST_ASSIGN: {
            generate_code_detail(env, ast->rhs);

            assert(ast->lhs->kind == AST_VAR);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "mov #eax, %d(#rbp)", ast->lhs->stack_idx);
            appcode(env->codes, "push #rax");
        } break;

        case AST_VAR:
            appcode(env->codes, "push %d(#rbp)", ast->stack_idx);
            break;

        case AST_FUNCCALL: {
            int i;

            for (i = vector_size(ast->args) - 1; i >= 0; i--) {
                generate_code_detail(env, (AST *)(vector_get(ast->args, i)));
                if (i < 6) appcode(env->codes, "pop %s", rreg[i]);
            }
            appcode(env->codes, "call %s", ast->fname);
            appcode(env->codes, "push #rax");
        } break;

        case AST_FUNCDEF: {
            int i, stack_idx;

            // allocate stack
            stack_idx = 0;
            for (i = max(0, vector_size(ast->params) - 6);
                 i < vector_size(ast->env->scoped_vars); i++) {
                stack_idx -= 4;
                ((AST *)(vector_get(ast->env->scoped_vars, i)))->stack_idx =
                    stack_idx;
            }

            // generate code
            appcode(env->codes, "%s:", ast->fname);
            appcode(env->codes, "push #rbp");
            appcode(env->codes, "mov #rsp, #rbp");
            appcode(env->codes, "sub $%d, #rsp",
                    (int)(ceil(-stack_idx / 8.)) * 8);

            // assign param to localvar
            for (i = 0; i < vector_size(ast->params); i++) {
                AST *var = lookup_var(
                    ast->env, (const char *)(vector_get(ast->params, i)));
                if (i < 6)
                    appcode(env->codes, "mov %s, %d(#rbp)", ereg[i],
                            var->stack_idx);
                else
                    // should avoid return pointer and saved %rbp
                    var->stack_idx = 16 + (i - 6) * 8;
            }

            // generate body
            generate_code_detail(env, ast->body);

            // avoid duplicate needless `ret`
            if (strcmp((const char *)vector_get(env->codes,
                                                vector_size(env->codes) - 1),
                       "ret") == 0)
                break;
            appcode(env->codes, "mov $0, #eax");
            appcode(env->codes, "mov #rbp, #rsp");
            appcode(env->codes, "pop #rbp");
            appcode(env->codes, "ret");
        } break;

        case AST_EXPR_STMT:
            if (ast->lhs == NULL) break;
            generate_code_detail(env, ast->lhs);
            appcode(env->codes, "pop #rax");
            break;

        case AST_RETURN:
            if (ast->lhs == NULL) {
                appcode(env->codes, "mov $0, #eax");
            }
            else {
                generate_code_detail(env, ast->lhs);
                appcode(env->codes, "pop #rax");
            }
            appcode(env->codes, "mov #rbp, #rsp");
            appcode(env->codes, "pop #rbp");
            appcode(env->codes, "ret");
            break;

        case AST_WHILE: {
            int org_loop_start_label = env->loop_continue_label,
                org_loop_end_label = env->loop_break_label;
            env->loop_continue_label = env->nlabel++;
            env->loop_break_label = env->nlabel++;

            appcode(env->codes, ".L%d:", env->loop_continue_label);
            assert(ast->cond != NULL);
            generate_code_detail(env, ast->cond);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp $0, #eax");
            appcode(env->codes, "je .L%d", env->loop_break_label);
            generate_code_detail(env, ast->then);
            appcode(env->codes, "jmp .L%d", env->loop_continue_label);
            appcode(env->codes, ".L%d:", env->loop_break_label);

            env->loop_continue_label = org_loop_start_label;
            env->loop_break_label = org_loop_end_label;
        } break;

        case AST_FOR: {
            int org_loop_start_label = env->loop_continue_label,
                org_loop_end_label = env->loop_break_label;
            int loop_start_label = env->nlabel++;
            env->loop_continue_label = env->nlabel++;
            env->loop_break_label = env->nlabel++;

            if (ast->initer != NULL) generate_code_detail(env, ast->initer);
            appcode(env->codes, ".L%d:", loop_start_label);
            if (ast->midcond != NULL) {
                generate_code_detail(env, ast->midcond);
                appcode(env->codes, "pop #rax");
                appcode(env->codes, "cmp $0, #eax");
                appcode(env->codes, "je .L%d", env->loop_break_label);
            }
            generate_code_detail(env, ast->for_body);
            appcode(env->codes, ".L%d:", env->loop_continue_label);
            if (ast->iterer != NULL) generate_code_detail(env, ast->iterer);
            appcode(env->codes, "jmp .L%d", loop_start_label);
            appcode(env->codes, ".L%d:", env->loop_break_label);

            env->loop_continue_label = org_loop_start_label;
            env->loop_break_label = org_loop_end_label;
        } break;

        case AST_BREAK:
            if (env->loop_break_label < 0)
                error("invalid break.", __FILE__, __LINE__);
            appcode(env->codes, "jmp .L%d", env->loop_break_label);
            break;

        case AST_CONTINUE:
            if (env->loop_continue_label < 0)
                error("invalid continue.", __FILE__, __LINE__);
            appcode(env->codes, "jmp .L%d", env->loop_continue_label);
            break;

        case AST_COMPOUND:
            generate_code(env, ast->stmts);
            break;

        case AST_INT:
            appcode(env->codes, "mov $%d, #eax", ast->ival);
            appcode(env->codes, "push #rax");
            break;

        case AST_NOP:
            break;

        default:
            assert(0);
    }
}

void generate_code(CodeEnv *env, Vector *asts)
{
    int i;
    for (i = 0; i < vector_size(asts); i++) {
        generate_code_detail(env, (AST *)vector_get(asts, i));
    }
}

#include "test.c"

int main(int argc, char **argv)
{
    Vector *tokens;
    TokenSeq *tokseq;
    Vector *asts;
    CodeEnv *env;

    if (argc == 2) {
        execute_test();
        return 0;
    }

    tokens = read_all_tokens(stdin);
    tokseq = new_token_seq(tokens);
    asts = parse_prog(NULL, tokseq);

    env = new_code_env();
    appcode(env->codes, ".global main");
    generate_code(env, asts);

    dump_codes(env->codes, stdout);

    return 0;
}
