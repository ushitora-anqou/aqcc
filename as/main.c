#include "aqcc.h"

#include "test.inc"

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "test") == 0) {
        execute_test();
        return 0;
    }

    if (argc != 3) goto usage;

    char *infile = argv[1], *outfile = argv[2];

    Vector *code = read_asm_from_filepath(infile);
    ObjectImage *obj = assemble_code(code);

    FILE *fh = fopen(outfile, "wb");
    dump_object_image(obj, fh);
    fclose(fh);

    return 0;

usage:
    error("Usage: as input-asm-file-path output-obj-file-path");
}
