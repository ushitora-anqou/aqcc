#include "aqcc.h"

Token *new_token(int kind, Source *source)
{
    Token *token = (Token *)safe_malloc(sizeof(Token));
    token->kind = kind;
    token->source = source;
    return token;
}

Token *clone_token(Token *src)
{
    Token *dst = (Token *)safe_malloc(sizeof(Token));
    memcpy(dst, src, sizeof(Token));
    return dst;
}

TokenSeq *new_token_seq(Vector *tokens)
{
    TokenSeq *tokseq = safe_malloc(sizeof(TokenSeq));
    tokseq->tokens = tokens;
    tokseq->idx = 0;
    return tokseq;
}

TokenSeq *tokenseq;

void init_tokenseq(Vector *tokens) { tokenseq = new_token_seq(tokens); }

void insert_tokens(Vector *tokens)
{
    Vector *tmp = new_vector();
    for (int i = 0; i < tokenseq->idx; i++)
        vector_push_back(tmp, vector_get(tokenseq->tokens, i));
    for (int i = 0; i < vector_size(tokens); i++) {
        Token *token = vector_get(tokens, i);
        if (token->kind == tEOF) break;
        vector_push_back(tmp, token);
    }
    for (int i = tokenseq->idx; i < vector_size(tokenseq->tokens); i++)
        vector_push_back(tmp, vector_get(tokenseq->tokens, i));
    tokenseq->tokens = tmp;
}

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
    TokenSeqSaved *tokseqsav;
    tokseqsav = (TokenSeqSaved *)safe_malloc(sizeof(TokenSeqSaved));
    tokseqsav->idx = tokenseq->idx;
    return tokseqsav;
}

void restore_token_seq_saved(TokenSeqSaved *saved)
{
    tokenseq->idx = saved->idx;
}
