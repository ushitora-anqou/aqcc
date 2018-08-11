#include "aqcc.h"

#include "test.inc"

char *read_entire_file(FILE *fh)
{
    // read the file all
    StringBuilder *sb = new_string_builder();
    int ch;
    while ((ch = fgetc(fh)) != EOF) string_builder_append(sb, ch);
    return string_builder_get(sb);
}

void erase_backslash_newline(char *src)
{
    char *r = src, *w = src;
    while (*r != '\0') {
        if (*r == '\\' && *(r + 1) == '\n')
            r += 2;
        else
            *w++ = *r++;
    }
    *w = '\0';
}

int main(int argc, char **argv)
{
    if (argc == 2) {
        execute_test();
        return 0;
    }

    char *prog_src = read_entire_file(stdin);
    erase_backslash_newline(prog_src);
    Vector *tokens = read_all_tokens(prog_src);
    tokens = preprocess_tokens(tokens);
    Vector *asts = parse_prog(tokens);
    analyze_ast(asts);
    Vector *code = generate_code(asts);

    dump_code(code, stdout);

    return 0;
}
