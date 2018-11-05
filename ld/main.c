#include "ld.h"

#include "test.inc"

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "test") == 0) {
        execute_test();
        return 0;
    }

    if (argc < 3) goto usage;

    Vector *objs = new_vector();
    for (int i = 1; i < argc - 1; i++) vector_push_back(objs, argv[i]);

    ExeImage *exe = link_objs(objs);

    FILE *fh = fopen(argv[argc - 1], "wb");
    dump_exe_image(exe, fh);
    fclose(fh);

    return 0;

usage:
    error("Usage: ld input-obj-file-path... output-exe-file-path");
}
