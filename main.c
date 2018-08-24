#include "aqcc.h"

#include "test.inc"

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "test") == 0) {
        execute_test();
        return 0;
    }

    if (argc == 4 && strcmp(argv[3], "experiment") == 0) {
        Vector *tokens = read_tokens_from_filepath(argv[1]);
        tokens = preprocess_tokens(tokens);
        tokens = concatenate_string_literal_tokens(tokens);

        Vector *asts = parse_prog(tokens);

        Env *env = analyze_ast(asts);
        optimize_asts_constant(asts, env);

        Vector *code = generate_register_code(asts);
        code = optimize_code(code);

        ObjectImage *obj = assemble_code(code);

        FILE *fh = fopen(argv[2], "wb");
        dump_object_image(obj, fh);
        fclose(fh);
        return 0;
    }

    if (argc != 2) error("Usage: aqcc FILEPATH");

    Vector *tokens = read_tokens_from_filepath(argv[1]);
    tokens = preprocess_tokens(tokens);
    tokens = concatenate_string_literal_tokens(tokens);

    Vector *asts = parse_prog(tokens);

    Env *env = analyze_ast(asts);
    optimize_asts_constant(asts, env);

    Vector *code = generate_register_code(asts);
    code = optimize_code(code);

    for (int i = 0; i < vector_size(code); i++)
        dump_code((Code *)vector_get(code, i), stdout);

    return 0;
}
