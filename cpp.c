#include "aqcc.h"

void preprocess_tokens_detail(Vector *tokens, int index, Vector *ntokens)
{
    assert(
}

Vector *preprocess_tokens(Vector *tokens)
{
    Vector *ntokens = new_vector();

    for (int i = 0; i < vector_size(tokens); i++) {
        Token *token = (Token *)vector_get(tokens, i);
        if (token->kind == tNUMBER)
            preprocess_tokens_detail(tokens, i, ntokens);
        else if (token->kind == tNEWLINE)
            continue;
        else
            vector_push_back(ntokens, token);
    }

    return ntokens;
}
