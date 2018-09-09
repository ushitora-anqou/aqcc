#include "aqcc.h"

void skip_newline()
{
    while (pop_token_if(tNEWLINE))
        ;
}

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

void preprocess_tokens_detail_define()
{
    char *name = expect_token(tIDENT)->sval;
    Vector *tokens = new_vector();
    while (!match_token(tNEWLINE)) vector_push_back(tokens, pop_token());
    expect_token(tNEWLINE);
    add_define(name, tokens);
}

void preprocess_tokens_detail_include()
{
    char *filepath = expect_token(tSTRING_LITERAL)->sval;
    expect_token(tNEWLINE);
    insert_tokens(read_tokens_from_filepath(filepath));
}

void preprocess_tokens_detail_ifdef_ifndef(const char *keyword)
{
    char *name = expect_token(tIDENT)->sval;
    expect_token(tNEWLINE);
    if (strcmp("ifdef", keyword) == 0 && lookup_define(name)) return;
    if (strcmp("ifndef", keyword) == 0 && !lookup_define(name)) return;

    // search corresponding #endif
    // TODO: #if
    int cnt = 1;
    while (1) {
        Token *token = pop_token();

        if (token->kind == tEOF) {
            if (strcmp("ifdef", keyword) == 0)
                error_unexpected_token_str("#endif corresponding to #ifdef",
                                           token);
            else
                error_unexpected_token_str("#endif corresponding to #ifndef",
                                           token);
        }

        if (token->kind == tNUMBER && (token = pop_token_if(tIDENT))) {
            char *ident = token->sval;
            if (strcmp("ifdef", ident) == 0 || strcmp("ifndef", ident) == 0)
                cnt++;
            else if (strcmp("endif", ident) == 0 && --cnt == 0)
                break;
        }
    }
    expect_token(tNEWLINE);
}

void preprocess_tokens_detail_number()
{
    char *keyword = expect_token(tIDENT)->sval;
    // TODO: other preprocess token
    if (strcmp(keyword, "define") == 0)
        preprocess_tokens_detail_define();
    else if (strcmp(keyword, "include") == 0)
        preprocess_tokens_detail_include();
    else if ((strcmp(keyword, "ifdef") == 0) || strcmp(keyword, "ifndef") == 0)
        preprocess_tokens_detail_ifdef_ifndef(keyword);
    else if (strcmp(keyword, "endif") == 0)
        return;
    else
        error("invalid preprocess token");
}

Vector *preprocess_tokens(Vector *tokens)
{
    init_tokenseq(tokens);
    init_define(tokens);

    Vector *ntokens = new_vector();
    while (!match_token(tEOF)) {
        Token *token = pop_token();
        if (token->kind == tNUMBER) {
            preprocess_tokens_detail_number();
            continue;
        }

        if (token->kind == tNEWLINE) continue;

        if (token->kind == tIDENT) {
            // TODO: should be implemented by function macro?
            // TODO: it can handle only `va_arg(args_var_name, int|char *)`
            if (strcmp(token->sval, "__builtin_va_arg") == 0) {
                Token *ntoken = clone_token(token);
                ntoken->sval = "__builtin_va_arg_int";
                vector_push_back(ntokens, ntoken);

                skip_newline();
                vector_push_back(ntokens, expect_token(tLPAREN));
                skip_newline();
                while (!match_token(tCOMMA))
                    vector_push_back(ntokens, pop_token());
                expect_token(tCOMMA);
                skip_newline();
                if (pop_token_if(kCHAR)) {
                    skip_newline();
                    expect_token(tSTAR);
                    ntoken->sval = "__builtin_va_arg_charp";
                }
                else {
                    expect_token(kINT);
                }
                skip_newline();
                vector_push_back(ntokens, expect_token(tRPAREN));
                continue;
            }

            Vector *deftokens = lookup_define(token->sval);
            if (deftokens != NULL) {  // found: replace tokens
                insert_tokens(deftokens);
                continue;
            }
        }

        vector_push_back(ntokens, token);
    }
    vector_push_back(ntokens, expect_token(tEOF));

    return ntokens;
}
