#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aqcc.h"

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

    if (strcmp(buf, "return") == 0) return new_token(tRETURN);

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

AST *new_funcdef_ast(char *fname, Vector *params, Vector *body)
{
    AST *ast = new_ast(AST_FUNCDEF);
    ast->fname = fname;
    ast->params = params;
    ast->body = body;
    return ast;
}

AST *parse_assignment_expr(TokenSeq *tokseq);

AST *parse_primary_expr(TokenSeq *tokseq)
{
    AST *ast;
    Token *token = pop_token(tokseq);
    switch (token->kind) {
        case tINT:
            ast = new_ast(AST_INT);
            ast->ival = token->ival;
            break;

        case tLPAREN:
            ast = parse_expr(tokseq);
            expect_token(tokseq, tRPAREN);
            break;

        case tIDENT:
            ast = new_ast(AST_VAR);
            ast->sval = token->sval;
            break;

        default:
            error("unexpected token", __FILE__, __LINE__);
    }
    return ast;
}

Vector *parse_argument_expr_list(TokenSeq *tokseq)
{
    Vector *args = new_vector();

    if (match_token(tokseq, tRPAREN)) return args;

    while (1) {
        vector_push_back(args, parse_assignment_expr(tokseq));
        if (match_token(tokseq, tRPAREN)) break;
        expect_token(tokseq, tCOMMA);
    }

    return args;
}

AST *parse_postfix_expr(TokenSeq *tokseq)
{
    AST *ast = parse_primary_expr(tokseq);
    Token *token = peek_token(tokseq);

    switch (token->kind) {
        case tLPAREN:
            if (ast->kind != AST_VAR)
                error("not func name", __FILE__, __LINE__);
            pop_token(tokseq);
            ast = new_funccall_ast(ast->sval, parse_argument_expr_list(tokseq));
            expect_token(tokseq, tRPAREN);
            break;
    }

    return ast;
}

AST *parse_unary_expr(TokenSeq *tokseq)
{
    Token *token = peek_token(tokseq);
    switch (token->kind) {
        case tPLUS:
            pop_token(tokseq);
            return parse_postfix_expr(tokseq);

        case tMINUS: {
            AST *ast;

            pop_token(tokseq);
            ast = new_ast(AST_UNARY_MINUS);
            ast->lhs = parse_postfix_expr(tokseq);
            return ast;
        }
    }
    return parse_postfix_expr(tokseq);
}

AST *parse_multiplicative_expr(TokenSeq *tokseq)
{
    AST *ast = parse_unary_expr(tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tSTAR:
                pop_token(tokseq);
                ast = new_binop_ast(AST_MUL, ast, parse_unary_expr(tokseq));
                break;
            case tSLASH:
                pop_token(tokseq);
                ast = new_binop_ast(AST_DIV, ast, parse_unary_expr(tokseq));
                break;
            case tPERCENT:
                pop_token(tokseq);
                ast = new_binop_ast(AST_REM, ast, parse_unary_expr(tokseq));
                break;
            default:
                return ast;
        }
    }
}

AST *parse_additive_expr(TokenSeq *tokseq)
{
    AST *ast = parse_multiplicative_expr(tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tPLUS:
                pop_token(tokseq);
                ast = new_binop_ast(AST_ADD, ast,
                                    parse_multiplicative_expr(tokseq));
                break;
            case tMINUS:
                pop_token(tokseq);
                ast = new_binop_ast(AST_SUB, ast,
                                    parse_multiplicative_expr(tokseq));
                break;
            default:
                return ast;
        }
    }
}

AST *parse_shift_expr(TokenSeq *tokseq)
{
    AST *ast = parse_additive_expr(tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tLSHIFT:
                pop_token(tokseq);
                ast =
                    new_binop_ast(AST_LSHIFT, ast, parse_additive_expr(tokseq));
                break;
            case tRSHIFT:
                pop_token(tokseq);
                ast =
                    new_binop_ast(AST_RSHIFT, ast, parse_additive_expr(tokseq));
                break;
            default:
                return ast;
        }
    }
}

AST *parse_relational_expr(TokenSeq *tokseq)
{
    AST *ast = parse_shift_expr(tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tLT:
                pop_token(tokseq);
                ast = new_binop_ast(AST_LT, ast, parse_shift_expr(tokseq));
                break;
            case tGT:
                pop_token(tokseq);
                ast = new_binop_ast(AST_GT, ast, parse_shift_expr(tokseq));
                break;
            case tLTE:
                pop_token(tokseq);
                ast = new_binop_ast(AST_LTE, ast, parse_shift_expr(tokseq));
                break;
            case tGTE:
                pop_token(tokseq);
                ast = new_binop_ast(AST_GTE, ast, parse_shift_expr(tokseq));
                break;
            default:
                return ast;
        }
    }
}

AST *parse_equality_expr(TokenSeq *tokseq)
{
    AST *ast = parse_relational_expr(tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tEQEQ:
                pop_token(tokseq);
                ast = new_binop_ast(AST_EQ, ast, parse_relational_expr(tokseq));
                break;
            case tNEQ:
                pop_token(tokseq);
                ast =
                    new_binop_ast(AST_NEQ, ast, parse_relational_expr(tokseq));
                break;
            default:
                return ast;
        }
    }
}

AST *parse_and_expr(TokenSeq *tokseq)
{
    AST *ast = parse_equality_expr(tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tAND) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_AND, ast, parse_equality_expr(tokseq));
    }
}

AST *parse_exclusive_or_expr(TokenSeq *tokseq)
{
    AST *ast = parse_and_expr(tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tHAT) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_XOR, ast, parse_and_expr(tokseq));
    }
}

AST *parse_inclusive_or_expr(TokenSeq *tokseq)
{
    AST *ast = parse_exclusive_or_expr(tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tBAR) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_OR, ast, parse_exclusive_or_expr(tokseq));
    }
}

AST *parse_logical_and_expr(TokenSeq *tokseq)
{
    AST *ast = parse_inclusive_or_expr(tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tANDAND) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_LAND, ast, parse_inclusive_or_expr(tokseq));
    }
}

AST *parse_logical_or_expr(TokenSeq *tokseq)
{
    AST *ast = parse_logical_and_expr(tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tBARBAR) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_LOR, ast, parse_logical_and_expr(tokseq));
    }
}

AST *parse_conditional_expr(TokenSeq *tokseq)
{
    AST *cond, *true_expr, *false_expr, *ast;

    cond = parse_logical_or_expr(tokseq);
    if (!match_token(tokseq, tQUESTION)) return cond;
    pop_token(tokseq);
    true_expr = parse_expr(tokseq);
    expect_token(tokseq, tCOLON);
    false_expr = parse_conditional_expr(tokseq);

    ast = new_ast(AST_COND);
    ast->cond = cond;
    ast->true_expr = true_expr;
    ast->false_expr = false_expr;
    return ast;
}

AST *parse_assignment_expr(TokenSeq *tokseq)
{
    Token *token;
    AST *ast;

    if (!match_token2(tokseq, tIDENT, tEQ))
        return parse_conditional_expr(tokseq);
    token = expect_token(tokseq, tIDENT);
    expect_token(tokseq, tEQ);

    ast = new_ast(AST_VAR);
    ast->sval = token->sval;
    return new_binop_ast(AST_ASSIGN, ast, parse_assignment_expr(tokseq));
}

AST *parse_expr(TokenSeq *tokseq) { return parse_assignment_expr(tokseq); }

AST *parse_expression_stmt(TokenSeq *tokseq)
{
    AST *expr = NULL, *stmt;

    if (!match_token(tokseq, tSEMICOLON)) expr = parse_expr(tokseq);
    expect_token(tokseq, tSEMICOLON);

    stmt = new_ast(AST_EXPR_STMT);
    stmt->lhs = expr;
    return stmt;
}

AST *parse_return_stmt(TokenSeq *tokseq)
{
    AST *ast = NULL;

    expect_token(tokseq, tRETURN);
    if (!match_token(tokseq, tSEMICOLON)) ast = parse_expr(tokseq);
    ast = new_binop_ast(AST_RETURN, ast, NULL);
    expect_token(tokseq, tSEMICOLON);

    return ast;
}

AST *parse_stmt(TokenSeq *tokseq)
{
    if (match_token(tokseq, tRETURN)) return parse_return_stmt(tokseq);
    return parse_expression_stmt(tokseq);
}

Vector *parse_compound_stmt(TokenSeq *tokseq)
{
    Vector *asts = new_vector();

    expect_token(tokseq, tLBRACE);
    while (peek_token(tokseq)->kind != tRBRACE) {
        vector_push_back(asts, parse_stmt(tokseq));
    }
    expect_token(tokseq, tRBRACE);

    return asts;
}

Vector *parse_parameter_list(TokenSeq *tokseq)
{
    Vector *params = new_vector();

    if (match_token(tokseq, tRPAREN)) return params;

    while (1) {
        vector_push_back(params, expect_token(tokseq, tIDENT)->sval);
        if (match_token(tokseq, tRPAREN)) break;
        expect_token(tokseq, tCOMMA);
    }

    return params;
}

AST *parse_function_definition(TokenSeq *tokseq)
{
    char *fname;
    Vector *params;

    fname = expect_token(tokseq, tIDENT)->sval;
    expect_token(tokseq, tLPAREN);
    params = parse_parameter_list(tokseq);
    expect_token(tokseq, tRPAREN);

    return new_funcdef_ast(fname, params, parse_compound_stmt(tokseq));
}

Vector *parse_prog(TokenSeq *tokseq)
{
    Vector *asts = new_vector();

    while (peek_token(tokseq)->kind != tEOF) {
        vector_push_back(asts, parse_function_definition(tokseq));
    }

    return asts;
}

typedef struct {
    int nlabel, stack_idx;
    Vector *codes;
    Map *var_map;
} CodeEnv;

CodeEnv *new_code_env()
{
    CodeEnv *this;

    this = (CodeEnv *)safe_malloc(sizeof(CodeEnv));
    this->nlabel = 0;
    this->stack_idx = 0;
    this->codes = new_vector();
    this->var_map = new_map();
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

        case AST_COND: {
            int false_label = env->nlabel++, exit_label = env->nlabel++;

            generate_code_detail(env, ast->cond);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp $0, #eax");
            appcode(env->codes, "je .L%d", false_label);
            generate_code_detail(env, ast->true_expr);
            appcode(env->codes, "jmp .L%d", exit_label);
            appcode(env->codes, ".L%d:", false_label);
            generate_code_detail(env, ast->false_expr);
            appcode(env->codes, ".L%d:", exit_label);
        } break;

        case AST_ASSIGN: {
            KeyValue *kv;

            generate_code_detail(env, ast->rhs);

            assert(ast->lhs->kind == AST_VAR);
            kv = map_lookup(env->var_map, ast->lhs->sval);
            if (kv == NULL) {  // not exists
                env->stack_idx -= 4;
                kv = map_insert(env->var_map, ast->lhs->sval,
                                new_int(env->stack_idx));
            }
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "mov #eax, %d(#rbp)", *(int *)(kv->value));
            appcode(env->codes, "push #rax");
        } break;

        case AST_VAR: {
            KeyValue *kv = map_lookup(env->var_map, ast->sval);
            if (kv == NULL) error("undefined variable.", __FILE__, __LINE__);
            appcode(env->codes, "push %d(#rbp)", *(int *)(kv->value));
            break;
        } break;

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
            CodeEnv *new_env = new_code_env();
            int i;

            appcode(env->codes, "%s:", ast->fname);
            appcode(env->codes, "push #rbp");
            appcode(env->codes, "mov #rsp, #rbp");

            for (i = 0; i < vector_size(ast->params); i++) {
                const char *pname = (const char *)(vector_get(ast->params, i));
                int stack_idx;

                if (i < 6) {
                    stack_idx = new_env->stack_idx - 4;
                    appcode(new_env->codes, "mov %s, %d(#rbp)", ereg[i],
                            stack_idx);
                }
                else {
                    // should avoid return pointer and saved %rbp
                    stack_idx = 16 + (i - 6) * 8;
                }

                new_env->stack_idx = stack_idx;
                map_insert(new_env->var_map, pname, new_int(stack_idx));
            }

            generate_code(new_env, ast->body);
            if (new_env->stack_idx != 0)
                appcode(env->codes, "sub $%d, #rsp",
                        (int)(ceil(-new_env->stack_idx / 8.)) * 8);
            for (i = 0; i < vector_size(new_env->codes); i++)
                vector_push_back(env->codes, vector_get(new_env->codes, i));

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
    asts = parse_prog(tokseq);

    env = new_code_env();
    appcode(env->codes, ".global main");
    generate_code(env, asts);

    dump_codes(env->codes, stdout);

    return 0;
}
