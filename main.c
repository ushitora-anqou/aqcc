#include "aqcc.h"

#include "test.inc"

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "test") == 0) {
        execute_test();
        return 0;
    }

    if (argc < 3) error("Usage: aqcc [-S] input-file-path output-file-path");

    // TODO: getopt
    char *infile = argv[1], *outfile = argv[2];
    enum { ASSM_MODE, OBJ_MODE };
    int mode = OBJ_MODE;
    if (argc == 4) {
        mode = ASSM_MODE;
        infile = argv[2];
        outfile = argv[3];
    }

    Vector *tokens = read_tokens_from_filepath(infile);
    tokens = preprocess_tokens(tokens);
    tokens = concatenate_string_literal_tokens(tokens);

    Vector *asts = parse_prog(tokens);

    Env *env = analyze_ast(asts);
    optimize_asts_constant(asts, env);

    Vector *code = generate_register_code(asts);
    code = optimize_code(code);

    FILE *fh = fopen(outfile, "wb");
    switch (mode) {
        case ASSM_MODE:
            for (int i = 0; i < vector_size(code); i++)
                dump_code((Code *)vector_get(code, i), fh);
            break;

        case OBJ_MODE: {
            ObjectImage *obj = assemble_code(code);
            dump_object_image(obj, fh);
        } break;

        default:
            assert(0);
    }
    fclose(fh);

    return 0;
}
