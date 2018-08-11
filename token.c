#include "aqcc.h"

Token *new_token(int kind, int line, int column)
{
    Token *token = safe_malloc(sizeof(Token));
    token->kind = kind;
    token->line = line;
    token->column = column;
    return token;
}

TokenSeq *new_token_seq(Vector *tokens)
{
    TokenSeq *this = safe_malloc(sizeof(TokenSeq));

    this->tokens = tokens;
    this->idx = 0;
    return this;
}

TokenSeq *tokenseq;

void init_tokenseq(Vector *tokens) { tokenseq = new_token_seq(tokens); }

Token *peek_token()
{
    Token *token = vector_get(tokenseq->tokens, tokenseq->idx);
    if (token == NULL) error("no next token.");
    return token;
}

Token *pop_token()
{
    Token *token = vector_get(tokenseq->tokens, tokenseq->idx++);
    if (token == NULL) error("no next token.");
    return token;
}

Token *expect_token(int kind)
{
    Token *token = pop_token(tokenseq);
    if (token->kind != kind) error_unexpected_token_kind(kind, token);
    return token;
}

int match_token(int kind)
{
    Token *token = peek_token(tokenseq);
    return token->kind == kind;
}

Token *pop_token_if(int kind)
{
    if (match_token(kind)) return pop_token();
    return NULL;
}

int match_token2(int kind0, int kind1)
{
    Token *token = peek_token(tokenseq);
    if (token->kind != kind0) return 0;
    tokenseq->idx++;
    token = peek_token(tokenseq);
    tokenseq->idx--;
    if (token->kind != kind1) return 0;
    return 1;
}

TokenSeqSaved *new_token_seq_saved()
{
    TokenSeqSaved *this;

    this = (TokenSeqSaved *)safe_malloc(sizeof(TokenSeqSaved));
    this->idx = tokenseq->idx;

    return this;
}

void restore_token_seq_saved(TokenSeqSaved *saved)
{
    tokenseq->idx = saved->idx;
}
