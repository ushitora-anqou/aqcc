#include <stdio.h>
#include <stdlib.h>

enum {
    tINT,
};

typedef struct {
    int kind;

    union {
        int ival;
    };
} Token;

Token read_next_token()
{
    char buf[256];  // TODO: enough length?
    int bufidx = 0;

    while (1) {
        int ch;

        ch = fgetc(stdin);
        if (ch == EOF) {
            Token token;

            buf[bufidx++] = '\0';
            token.kind = tINT;
            token.ival = atoi(buf);
            return token;
        }

        buf[bufidx++] = ch;
    }
}

int main()
{
    Token token;

    token = read_next_token();

    puts(".global main");
    puts("main:");
    printf("\tmov $%d, %%eax\n", token.ival);
    puts("\tret");

    return 0;
}
