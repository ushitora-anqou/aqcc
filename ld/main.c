#include "aqcc.h"

#include "test.inc"

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "test") == 0) {
        execute_test();
        return 0;
    }

    if (argc < 3) goto usage;

    char *option = argv[1];

    if (strcmp(option, "cs") == 0) {  // compile
        if (argc != 4)
            error("Usage: aqcc cs input-c-file-path output-asm-file-path");

        char *infile = argv[2], *outfile = argv[3];

        Vector *tokens = read_tokens_from_filepath(infile);
        tokens = preprocess_tokens(tokens);
        tokens = concatenate_string_literal_tokens(tokens);

        Vector *asts = parse_prog(tokens);

        Env *env = analyze_ast(asts);
        optimize_asts_constant(asts, env);

        Vector *code = x86_64_generate_code(asts);
        code = optimize_code(code);

        FILE *fh = fopen(outfile, "wb");
        for (int i = 0; i < vector_size(code); i++)
            dump_code((Code *)vector_get(code, i), fh);
        fclose(fh);

        return 0;
    }

    if (strcmp(option, "so") == 0) {  // assemble
        if (argc != 4)
            error("Usage: aqcc so input-asm-file-path output-obj-file-path");

        char *infile = argv[2], *outfile = argv[3];

        Vector *code = read_asm_from_filepath(infile);
        ObjectImage *obj = assemble_code(code);

        FILE *fh = fopen(outfile, "wb");
        dump_object_image(obj, fh);
        fclose(fh);

        return 0;
    }

    if (strcmp(option, "oe") == 0) {  // link
        if (argc < 4)
            error("Usage: aqcc oe input-obj-file-path... output-exe-file-path");

        Vector *objs = new_vector();
        for (int i = 2; i < argc - 1; i++) vector_push_back(objs, argv[i]);

        ExeImage *exe = link_objs(objs);

        FILE *fh = fopen(argv[argc - 1], "wb");
        dump_exe_image(exe, fh);
        fclose(fh);

        return 0;
    }

usage:
    error("Usage: aqcc [cs|so|oe] input-file-path... output-file-path");
}
