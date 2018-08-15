#include "aqcc.h"

#include "test.inc"

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3) error("Usage: aqcc FILEPATH");

    if (strcmp(argv[1], "test") == 0) {
        execute_test();
        return 0;
    }

    Vector *tokens = read_tokens_from_filepath(argv[1]);
    tokens = preprocess_tokens(tokens);
    tokens = concatenate_string_literal_tokens(tokens);
    Vector *asts = parse_prog(tokens);
    analyze_ast(asts);
    Vector *code;
    code = generate_register_code(asts);

    dump_code(code, stdout);

    return 0;
}
