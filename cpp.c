#include "aqcc.h"

Map *define_table;

void init_define() { define_table = new_map(); }

Vector *add_define(char *name, Vector *tokens)
{
    if (map_lookup(define_table, name))
        error("duplicate define's name: '%s'", name);
    map_insert(define_table, name, tokens);
    return tokens;
}

Vector *lookup_define(char *name)
{
    return (Vector *)kv_value(map_lookup(define_table, name));
}

void preprocess_tokens_detail_number(Vector *ntokens)
{
    char *keyword = expect_token(tIDENT)->sval;
    // TODO: other preprocess token
    if (strcmp(keyword, "define") != 0) error("invalid preprocess token");
    char *name = expect_token(tIDENT)->sval;
    Vector *tokens = new_vector();
    while (!match_token(tNEWLINE)) vector_push_back(tokens, pop_token());
    pop_token();
    add_define(name, tokens);
}

Vector *preprocess_tokens(Vector *tokens)
{
    init_tokenseq(tokens);
    init_define(tokens);

    Vector *ntokens = new_vector();
    while (!match_token(tEOF)) {
        Token *token = pop_token();
        if (token->kind == tNUMBER) {
            preprocess_tokens_detail_number(ntokens);
            continue;
        }

        if (token->kind == tNEWLINE) continue;

        if (token->kind == tIDENT) {
            Vector *tokens = lookup_define(token->sval);
            if (tokens != NULL) {  // found: replace tokens
                vector_push_back_vector(ntokens, tokens);
                continue;
            }
        }

        vector_push_back(ntokens, token);
    }
    vector_push_back(ntokens, expect_token(tEOF));

    return ntokens;
}
