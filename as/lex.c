#include "as.h"

Source source;

void init_source(char *src, char *filepath)
{
    // caluculate current working directory
    int i, j, len = strlen(filepath);
    source.cwd = safe_malloc(sizeof(char *) * len);
    // TODO: ad-hoc
    for (i = len - 1; i >= 0; i--) {
        if (filepath[i] == '/') {
            // detect last '/'
            for (j = 0; j <= i; j++) {
                source.cwd[j] = filepath[j];
            }
            source.cwd[i + 1] = '\0';
            break;
        }
    }
    // When this source code and aqcc are located in the same directory
    if (source.cwd[0] == '\0') {
        strcpy(source.cwd, "./");
    }

    source.filepath = filepath;
    source.src = src;
    source.line = source.column = 1;
    source.line2length = new_vector();

    // fill file.line2length
    vector_push_back(source.line2length, NULL);  // line is 1-based index.
    for (int i = 0, column = 1; src[i] == '\0'; i++, column++) {
        if (src[i] == '\n') {
            vector_push_back(source.line2length, (void *)column);
            column = 0;
        }
    }
}

void ungetch()
{
    source.src--;
    if (*source.src == '\n') {
        source.line--;
        source.column = (int)vector_get(source.line2length, source.line);
    }
    else {
        source.column--;
    }
}

char peekch() { return *source.src; }

char getch()
{
    char ch = peekch();
    if (ch == '\0') error("unexpected EOF");

    if (ch == '\n') {
        source.line++;
        source.column = 0;
    }
    else {
        source.column++;
    }
    return *source.src++;
}

int read_next_hex_int()
{
    // assume that prefix '0x' is already read.
    int acc = 0;
    while (1) {
        int ch = peekch();
        if (isdigit(ch))
            acc = 16 * acc + ch - '0';
        else if ('a' <= ch && ch <= 'f')
            acc = 16 * acc + 10 + ch - 'a';
        else if ('A' <= ch && ch <= 'F')
            acc = 16 * acc + 10 + ch - 'A';
        else
            break;
        getch();
    }

    return acc;
}

int read_next_dec_int()
{
    int acc = 0;
    while (isdigit(peekch())) acc = 10 * acc + getch() - '0';
    return acc;
}

int read_next_oct_int()
{
    // assume that prefix '0' is already read.
    int acc = 0;
    while (1) {
        int ch = peekch();
        if ('0' <= ch && ch <= '7')
            acc = 8 * acc + ch - '0';
        else
            break;
        getch();
    }

    return acc;
}

int read_next_int()
{
    int ch = peekch(), ival;
    if (ch == '0') {
        getch();
        if (peekch() == 'x') {
            getch();
            ival = read_next_hex_int();
        }
        else
            ival = read_next_oct_int();
    }
    else
        ival = read_next_dec_int();

    return ival;
}

// assume that the first doublequote has been already read.
void read_next_string_literal(char **sval, int *ssize)
{
    StringBuilder *sb = new_string_builder();
    while (1) {
        char ch = getch();

        switch (ch) {
            case '\\':
                ch = getch();
                ch = unescape_char(ch);
                break;

            case '"':
                goto end;

            case '\n':
                error("unexpected new-line character");
        }

        string_builder_append(sb, ch);
    }

end:
    *ssize = string_builder_size(sb);
    *sval = string_builder_get(sb);
}

void skip_space()
{
    while (isspace(peekch())) getch();
}

char speekch()
{
    skip_space();
    return peekch();
}

char sgetch()
{
    skip_space();
    return getch();
}

_Noreturn void unexpected_char_error(char expect, char got)
{
    error("%s:%d:%d: unexpected character: expect %c, got %c", source.filepath,
          source.line, source.column, expect, got);
}

char sexpect_ch(char expect)
{
    int ch = sgetch();
    if (ch != expect) unexpected_char_error(expect, ch);
    return ch;
}

int read_asm_ival()
{
    int mul = 1;
    if (speekch() == '-') {
        getch();
        mul = -1;
    }
    return read_next_int() * mul;
}

char *read_asm_token()
{
    StringBuilder *sb = new_string_builder();
    string_builder_append(sb, sgetch());
    while (1) {
        int ch = peekch();
        if (!isalnum(ch) && ch != '_' && ch != '.' && ch != ':') break;
        string_builder_append(sb, getch());
    }
    char *str = string_builder_get(sb);

    return str;
}

Code *read_asm_memory()
{
    sexpect_ch('(');
    char *reg = read_asm_token();
    sexpect_ch(')');
    return str2reg(reg);
}

Code *read_asm_param()
{
    char ch = speekch();
    switch (ch) {
        case '%':
            return str2reg(read_asm_token());

        case '$': {
            getch();  // already skipped space
            return new_value_code(read_asm_ival());
        }

        case '(':
            return new_addrof_code(read_asm_memory(), 0);
    }

    if (isdigit(ch) || ch == '-') {
        int offset = read_asm_ival();
        return new_addrof_code(read_asm_memory(), offset);
    }

    char *label = read_asm_token();
    return new_addrof_label_code(read_asm_memory(), label);
}

Vector *read_all_asm(char *src, char *filepath)
{
    init_source(src, filepath);

    Vector *code = new_vector();

    while (speekch() != '\0') {
        // comment
        if (peekch() == '/') {
            getch();
            if (getch() == '*') {
                // begin comment
                while (1) {
                    if (getch() != '*') continue;
                    if (getch() == '/') break;  // end comment
                    ungetch();
                }
                continue;
            }
            else
                ungetch();
        }

        char *str = read_asm_token();
        int len = strlen(str);

        if (str[len - 1] == ':') {  // label
            str[len - 1] = '\0';
            vector_push_back(code, LABEL(str));
            continue;
        }

        if (strcmp(str, ".ascii") == 0) {
            sexpect_ch('"');
            char *sval;
            int ssize;
            read_next_string_literal(&sval, &ssize);

            Code *c = new_code(CD_ASCII);
            c->sval = sval;
            c->ival = ssize - 1;
            vector_push_back(code, c);
            continue;
        }

        KeyValue *kv;

        Map *binop_table = new_map();
        map_insert(binop_table, "mov", (void *)INST_MOV);
        map_insert(binop_table, "movl", (void *)INST_MOVL);
        map_insert(binop_table, "movsbl", (void *)INST_MOVSBL);
        map_insert(binop_table, "movslq", (void *)INST_MOVSLQ);
        map_insert(binop_table, "movzb", (void *)INST_MOVZB);
        map_insert(binop_table, "lea", (void *)INST_LEA);
        map_insert(binop_table, "add", (void *)INST_ADD);
        map_insert(binop_table, "add", (void *)INST_ADD);
        map_insert(binop_table, "addq", (void *)INST_ADDQ);
        map_insert(binop_table, "sub", (void *)INST_SUB);
        map_insert(binop_table, "imul", (void *)INST_IMUL);
        map_insert(binop_table, "sar", (void *)INST_SAR);
        map_insert(binop_table, "sal", (void *)INST_SAL);
        map_insert(binop_table, "cmp", (void *)INST_CMP);
        map_insert(binop_table, "and", (void *)INST_AND);
        map_insert(binop_table, "xor", (void *)INST_XOR);
        map_insert(binop_table, "or", (void *)INST_OR);
        if (kv = map_lookup(binop_table, str)) {
            Code *lhs = read_asm_param();
            sexpect_ch(',');
            Code *rhs = read_asm_param();
            vector_push_back(code, new_binop_code((int)kv_value(kv), lhs, rhs));
            continue;
        }

        Map *unary_table = new_map();
        map_insert(unary_table, "push", (void *)INST_PUSH);
        map_insert(unary_table, "pop", (void *)INST_POP);
        map_insert(unary_table, "idiv", (void *)INST_IDIV);
        map_insert(unary_table, "neg", (void *)INST_NEG);
        map_insert(unary_table, "not", (void *)INST_NOT);
        map_insert(unary_table, "setl", (void *)INST_SETL);
        map_insert(unary_table, "setle", (void *)INST_SETLE);
        map_insert(unary_table, "sete", (void *)INST_SETE);
        map_insert(unary_table, "incl", (void *)INST_INCL);
        map_insert(unary_table, "incq", (void *)INST_INCQ);
        map_insert(unary_table, "decl", (void *)INST_DECL);
        map_insert(unary_table, "decq", (void *)INST_DECQ);
        if (kv = map_lookup(unary_table, str)) {
            Code *lhs = read_asm_param();
            vector_push_back(code, new_unary_code((int)kv_value(kv), lhs));
            continue;
        }

        Map *simple_table = new_map();
        map_insert(simple_table, "ret", (void *)INST_RET);
        map_insert(simple_table, "nop", (void *)INST_NOP);
        map_insert(simple_table, "syscall", (void *)INST_SYSCALL);
        map_insert(simple_table, "cltd", (void *)INST_CLTD);
        map_insert(simple_table, "cltq", (void *)INST_CLTQ);
        map_insert(simple_table, ".text", (void *)CD_TEXT);
        map_insert(simple_table, ".data", (void *)CD_DATA);
        if (kv = map_lookup(simple_table, str)) {
            vector_push_back(code, new_code((int)kv_value(kv)));
            continue;
        }

        Map *label_table = new_map();
        map_insert(label_table, "call", (void *)INST_CALL);
        map_insert(label_table, "jmp", (void *)INST_JMP);
        map_insert(label_table, "je", (void *)INST_JE);
        map_insert(label_table, "jne", (void *)INST_JNE);
        map_insert(label_table, "jae", (void *)INST_JAE);
        map_insert(label_table, ".global", (void *)CD_GLOBAL);
        if (kv = map_lookup(label_table, str)) {
            char *label = read_asm_token();
            Code *c = new_code((int)kv_value(kv));
            c->label = label;
            vector_push_back(code, c);
            continue;
        }

        Map *ival_table = new_map();
        map_insert(ival_table, ".zero", (void *)CD_ZERO);
        map_insert(ival_table, ".long", (void *)CD_LONG);
        map_insert(ival_table, ".byte", (void *)CD_BYTE);
        map_insert(ival_table, ".quad", (void *)CD_QUAD);
        if (kv = map_lookup(ival_table, str)) {
            skip_space();
            int ival = read_asm_ival();
            Code *c = new_code((int)kv_value(kv));
            c->ival = ival;
            vector_push_back(code, c);
            continue;
        }

        error("%s:%d:%d: not implemented assembly: %s", source.filepath,
              source.line, source.column, str);
    }

    return code;
}

Vector *read_asm_from_filepath(char *filepath)
{
    char *src = read_entire_file(filepath);
    return read_all_asm(src, filepath);
}
