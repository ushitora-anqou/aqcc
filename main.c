#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "aqcc.h"

enum {
    tINT,
    tPLUS,
    tMINUS,
    tSTAR,
    tSLASH,
    tPERCENT,
    tLPAREN,
    tRPAREN,
    tLSHIFT,
    tRSHIFT,
    tEOF,
};

typedef struct {
    int kind;

    union {
        int ival;
    };
} Token;

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
                if (ch != '<') error("unexpected token", __FILE__, __LINE__);
                return new_token(tLSHIFT);
            case '>':
                ch = fgetc(fh);
                if (ch != '>') error("unexpected token", __FILE__, __LINE__);
                return new_token(tRSHIFT);
            case EOF:
                return new_token(tEOF);
        }
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

        default:
            error("unexpected token", __FILE__, __LINE__);
    }
    return ast;
}

AST *parse_unary_expr(TokenSeq *tokseq)
{
    Token *token = peek_token(tokseq);
    switch (token->kind) {
        case tPLUS:
            pop_token(tokseq);
            return parse_primary_expr(tokseq);

        case tMINUS: {
            AST *ast;

            pop_token(tokseq);
            ast = new_ast(AST_UNARY_MINUS);
            ast->lhs = parse_primary_expr(tokseq);
            return ast;
        }
    }
    return parse_primary_expr(tokseq);
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

AST *parse_expr(TokenSeq *tokseq) { return parse_shift_expr(tokseq); }

void print_code(FILE *fh, AST *ast)
{
    switch (ast->kind) {
        case AST_ADD:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rdi\n"
                    "pop %%rax\n"
                    "add %%rdi, %%rax\n"
                    "push %%rax\n");
            break;
        case AST_SUB:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rdi\n"
                    "pop %%rax\n"
                    "sub %%rdi, %%rax\n"
                    "push %%rax\n");
            break;
        case AST_MUL:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rdi\n"
                    "pop %%rax\n"
                    "imul %%rdi, %%rax\n"
                    "push %%rax\n");
            break;
        case AST_DIV:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rdi\n"
                    "pop %%rax\n"
                    "cqto\n"
                    "idiv %%rdi\n"
                    "push %%rax\n");
            break;
        case AST_REM:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rdi\n"
                    "pop %%rax\n"
                    "cqto\n"
                    "idiv %%rdi\n"
                    "push %%rdx\n");
            break;
        case AST_UNARY_MINUS:
            print_code(fh, ast->lhs);
            fprintf(fh,
                    "pop %%rax\n"
                    "neg %%rax\n"
                    "push %%rax\n");
            break;
        case AST_LSHIFT:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rcx\n"
                    "pop %%rax\n"
                    "sal %%cl, %%rax\n"
                    "push %%rax\n");
            break;
        case AST_RSHIFT:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rcx\n"
                    "pop %%rax\n"
                    "sar %%cl, %%rax\n"
                    "push %%rax\n");
            break;
        case AST_INT:
            fprintf(fh, "push $%d\n", ast->ival);
            break;
        default:
            assert(0);
    }
}

int main()
{
    Vector *tokens = read_all_tokens(stdin);
    TokenSeq *tokseq = new_token_seq(tokens);
    AST *ast = parse_expr(tokseq);

    puts(".global main");
    puts("main:");

    print_code(stdout, ast);

    puts("pop %rax");
    puts("ret");

    return 0;
}
