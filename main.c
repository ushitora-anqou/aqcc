#include "aqcc.h"

#include "test.inc"

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "test") == 0) {
        execute_test();
        return 0;
    }

    if (argc < 3) error("Usage: aqcc input-file-path output-file-path");

    // TODO: getopt
    char *infile = argv[1], *outfile = argv[2];

    // detect filetype by file extensions, like gcc :smile:
    enum { FT_UNK = -1, FT_C, FT_ASM, FT_OBJ };
    int in_ft = FT_UNK, out_ft = FT_UNK;
    char ft_table[128];                      // TODO: enough length?
    memset(ft_table, -1, sizeof(ft_table));  // -1 is FT_UNK
    ft_table['c'] = FT_C;
    ft_table['s'] = FT_ASM;
    ft_table['o'] = FT_OBJ;

    in_ft = ft_table[infile[strlen(infile) - 1]];
    out_ft = ft_table[outfile[strlen(outfile) - 1]];

    if (in_ft == FT_UNK || out_ft == FT_UNK)
        error("unknown (or may be invalid) input or output file type");
    if (out_ft == FT_C) error("invalid output file type");
    if (in_ft == FT_ASM || in_ft == FT_OBJ)
        error("invalid input file type (but will be valid :muscle:)");

    Vector *code;  // assembly

    if (in_ft == FT_C) {
        // compile
        Vector *tokens = read_tokens_from_filepath(infile);
        tokens = preprocess_tokens(tokens);
        tokens = concatenate_string_literal_tokens(tokens);

        Vector *asts = parse_prog(tokens);

        Env *env = analyze_ast(asts);
        optimize_asts_constant(asts, env);

        code = generate_register_code(asts);
        code = optimize_code(code);
    }

    FILE *fh = fopen(outfile, "wb");
    switch (out_ft) {
        case FT_ASM:
            for (int i = 0; i < vector_size(code); i++)
                dump_code((Code *)vector_get(code, i), fh);
            break;

        case FT_OBJ: {
            // assemble
            ObjectImage *obj = assemble_code(code);
            dump_object_image(obj, fh);
        } break;
    }
    fclose(fh);

    return 0;
}
