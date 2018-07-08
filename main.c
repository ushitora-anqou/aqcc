#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    tINT,
    tPLUS,
    tMINUS,
    tEOF,
};

typedef struct {
    int kind;

    union {
        int ival;
    };
} Token;

Token read_next_int_token(FILE *fh)
{
    char buf[256];  // TODO: enough length?
    int bufidx = 0;

    while (1) {
        int ch = fgetc(fh);

        if (!isdigit(ch)) {
            Token token;

            buf[bufidx++] = '\0';
            ungetc(ch, fh);
            token.kind = tINT;
            token.ival = atoi(buf);
            return token;
        }

        buf[bufidx++] = ch;
    }
}

Token read_next_token(FILE *fh)
{
    while (1) {
        int ch;

        ch = fgetc(fh);

        if (isdigit(ch)) {
            ungetc(ch, fh);
            return read_next_int_token(fh);
        }

        Token token;
        switch (ch) {
            case '+':
                token.kind = tPLUS;
                return token;
            case '-':
                token.kind = tMINUS;
                return token;
            case EOF:
                token.kind = tEOF;
                return token;
        }
    }
}

int main()
{
    puts(".global main");
    puts("main:");

    Token token = read_next_token(stdin);
    assert(token.kind == tINT);
    printf("push $%d\n", token.ival);

    while (1) {
        token = read_next_token(stdin);
        if (token.kind == tEOF) break;

        switch (token.kind) {
            case tPLUS:
                token = read_next_token(stdin);
                assert(token.kind == tINT);
                puts("pop %rax");
                printf("add $%d, %%eax\n", token.ival);
                puts("push %rax");
                break;

            case tMINUS:
                token = read_next_token(stdin);
                assert(token.kind == tINT);
                puts("pop %rax");
                printf("sub $%d, %%eax\n", token.ival);
                puts("push %rax");
                break;

            default:
                assert(0);
        }
    }

    puts("pop %rax");
    puts("ret");

    return 0;
}
