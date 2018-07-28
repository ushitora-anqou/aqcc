#include "aqcc.h"

#include "test.inc"

int main(int argc, char **argv)
{
    Vector *tokens, *asts, *codes;

    if (argc == 2) {
        execute_test();
        return 0;
    }

    tokens = read_all_tokens(stdin);
    asts = parse_prog(tokens);

    codes = generate_code(asts);

    dump_codes(codes, stdout);

    return 0;
}
