#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "aqcc.h"

enum {
    tINT,
    tPLUS,
    tMINUS,
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

        if (isdigit(ch)) {
            ungetc(ch, fh);
            return read_next_int_token(fh);
        }

        switch (ch) {
            case '+':
                return new_token(tPLUS);
            case '-':
                return new_token(tMINUS);
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

typedef struct {
    Vector *tokens;
    size_t idx;
} TokenSeq;

TokenSeq *new_token_seq(Vector *tokens)
{
    TokenSeq *this = safe_malloc(sizeof(TokenSeq));

    this->tokens = tokens;
    this->idx = 0;
    return this;
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

int main()
{
    Vector *tokens = read_all_tokens(stdin);
    TokenSeq *tokseq = new_token_seq(tokens);
    Token *token;

    puts(".global main");
    puts("main:");

    token = expect_token(tokseq, tINT);
    printf("push $%d\n", token->ival);

    while (1) {
        token = pop_token(tokseq);
        if (token->kind == tEOF) break;

        switch (token->kind) {
            case tPLUS:
                token = expect_token(tokseq, tINT);
                puts("pop %rax");
                printf("add $%d, %%eax\n", token->ival);
                puts("push %rax");
                break;

            case tMINUS:
                token = expect_token(tokseq, tINT);
                puts("pop %rax");
                printf("sub $%d, %%eax\n", token->ival);
                puts("push %rax");
                break;

            default:
                error("plus or minus is expected here.", __FILE__, __LINE__);
        }
    }

    puts("pop %rax");
    puts("ret");

    return 0;
}
