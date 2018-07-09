#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
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
                switch (ch) {
                    case '<':
                        return new_token(tLSHIFT);
                    case '=':
                        return new_token(tLTE);
                }
                return new_token(tLT);
            case '>':
                ch = fgetc(fh);
                switch (ch) {
                    case '>':
                        return new_token(tRSHIFT);
                    case '=':
                        return new_token(tGTE);
                }
                return new_token(tGT);
            case '=':
                ch = fgetc(fh);
                if (ch != '=') break;
                return new_token(tEQ);
            case '!':
                ch = fgetc(fh);
                if (ch != '=') break;
                return new_token(tNEQ);
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
            case tEQ:
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

AST *parse_expr(TokenSeq *tokseq) { return parse_equality_expr(tokseq); }

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
        case AST_LT:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rdi\n"
                    "pop %%rax\n"
                    "cmp %%rdi, %%rax\n"
                    "setl %%al\n"
                    "movzb %%al, %%eax\n"
                    "push %%rax\n");
            break;
        case AST_GT:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rdi\n"
                    "pop %%rax\n"
                    "cmp %%rdi, %%rax\n"
                    "setg %%al\n"
                    "movzb %%al, %%eax\n"
                    "push %%rax\n");
            break;
        case AST_LTE:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rdi\n"
                    "pop %%rax\n"
                    "cmp %%rdi, %%rax\n"
                    "setle %%al\n"
                    "movzb %%al, %%eax\n"
                    "push %%rax\n");
            break;
        case AST_GTE:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rdi\n"
                    "pop %%rax\n"
                    "cmp %%rdi, %%rax\n"
                    "setge %%al\n"
                    "movzb %%al, %%eax\n"
                    "push %%rax\n");
            break;
        case AST_EQ:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rdi\n"
                    "pop %%rax\n"
                    "cmp %%rdi, %%rax\n"
                    "sete %%al\n"
                    "movzb %%al, %%eax\n"
                    "push %%rax\n");
            break;
        case AST_NEQ:
            print_code(fh, ast->lhs);
            print_code(fh, ast->rhs);
            fprintf(fh,
                    "pop %%rdi\n"
                    "pop %%rax\n"
                    "cmp %%rdi, %%rax\n"
                    "setne %%al\n"
                    "movzb %%al, %%eax\n"
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
