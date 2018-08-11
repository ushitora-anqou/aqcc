#include "aqcc.h"

void preprocess_tokens_detail(Vector *ntokens)
{
    // assert(
}

Vector *preprocess_tokens(Vector *tokens)
{
    init_tokenseq(tokens);

    Vector *ntokens = new_vector();
    while (!match_token(tEOF)) {
        Token *token = pop_token();
        if (token->kind == tNUMBER)
            preprocess_tokens_detail(ntokens);
        else if (token->kind == tNEWLINE)
            continue;
        else
            vector_push_back(ntokens, token);
    }
    vector_push_back(ntokens, expect_token(tEOF));

    return ntokens;
}
