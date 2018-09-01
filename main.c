#include "aqcc.h"

#include "test.inc"

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "test") == 0) {
        execute_test();
        return 0;
    }

    if (argc != 3) error("Usage: aqcc input-file-path output-file-path");

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
