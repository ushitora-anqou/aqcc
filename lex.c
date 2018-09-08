#include "aqcc.h"

typedef struct {
    int line, column;
    Vector *line2length;
    char *src;
} Source;
Source source;

void init_source(char *src)
{
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

Token *make_token(int kind)
{
    return new_token(kind, source.line, source.column);
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

Token *read_next_int_token()
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

    Token *token = make_token(tINT);
    token->ival = ival;
    return token;
}

Token *read_next_ident_token()
{
    StringBuilder *sb = new_string_builder();
    while (1) {
        int ch = getch();

        if (!isalnum(ch) && ch != '_') {
            ungetch();
            break;
        }

        string_builder_append(sb, ch);
    }

    static Map *str2keyword = NULL;
    if (str2keyword == NULL) {
        str2keyword = new_map();

        map_insert(str2keyword, "return", (void *)kRETURN);
        map_insert(str2keyword, "if", (void *)kIF);
        map_insert(str2keyword, "else", (void *)kELSE);
        map_insert(str2keyword, "while", (void *)kWHILE);
        map_insert(str2keyword, "break", (void *)kBREAK);
        map_insert(str2keyword, "continue", (void *)kCONTINUE);
        map_insert(str2keyword, "for", (void *)kFOR);
        map_insert(str2keyword, "int", (void *)kINT);
        map_insert(str2keyword, "char", (void *)kCHAR);
        map_insert(str2keyword, "sizeof", (void *)kSIZEOF);
        map_insert(str2keyword, "switch", (void *)kSWITCH);
        map_insert(str2keyword, "default", (void *)kDEFAULT);
        map_insert(str2keyword, "case", (void *)kCASE);
        map_insert(str2keyword, "goto", (void *)kGOTO);
        map_insert(str2keyword, "struct", (void *)kSTRUCT);
        map_insert(str2keyword, "typedef", (void *)kTYPEDEF);
        map_insert(str2keyword, "do", (void *)kDO);
        map_insert(str2keyword, "void", (void *)kVOID);
        map_insert(str2keyword, "union", (void *)kUNION);
        map_insert(str2keyword, "const", (void *)kCONST);
        map_insert(str2keyword, "enum", (void *)kENUM);
        map_insert(str2keyword, "_Noreturn", (void *)kNORETURN);
        map_insert(str2keyword, "static", (void *)kSTATIC);
        map_insert(str2keyword, "extern", (void *)kEXTERN);
    }

    char *str;
    str = string_builder_get(sb);
    KeyValue *kv = map_lookup(str2keyword, str);
    if (kv) return make_token((int)kv_value(kv));

    Token *token = make_token(tIDENT);
    token->sval = str;
    return token;
}

// assume that the first doublequote has been already read.
Token *read_next_string_literal_token()
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

    Token *token;
end:
    token = make_token(tSTRING_LITERAL);
    token->sval = string_builder_get(sb);
    token->ssize = string_builder_size(sb);
    return token;
}

// assume that the first singlequote has been already read.
Token *read_next_character_constant_token()
{
    char ch = getch();
    if (ch == '\'') error("unexpected singlequote.");
    if (ch == '\\') ch = unescape_char(getch());
    while (getch() != '\'')
        ;

    Token *token = make_token(tINT);
    token->ival = ch;
    return token;
}

Token *read_next_token()
{
    while (peekch() != '\0') {
        char ch = getch();

        // \n should be a token because \n has some meaning in preprocessing.
        if (isspace(ch) && ch != '\n') continue;

        if (isdigit(ch)) {
            ungetch();
            return read_next_int_token();
        }

        if (isalpha(ch) || ch == '_') {
            ungetch();
            Token *token = read_next_ident_token();
            // TODO: for now, const is the same as comments.
            if (token->kind == kCONST) continue;
            // TODO: for now, _Noreturn is the same as comments.
            if (token->kind == kNORETURN) continue;
            return token;
        }

        switch (ch) {
            case '"':
                return read_next_string_literal_token();

            case '\'':
                return read_next_character_constant_token();

            case '+':
                ch = getch();
                if (ch == '+') return make_token(tINC);
                if (ch == '=') return make_token(tPLUSEQ);
                ungetch();
                return make_token(tPLUS);

            case '-':
                ch = getch();
                if (ch == '=') return make_token(tMINUSEQ);
                if (ch == '>') return make_token(tARROW);
                if (ch == '-') return make_token(tDEC);
                ungetch();
                return make_token(tMINUS);

            case '*':
                ch = getch();
                if (ch == '=') return make_token(tSTAREQ);
                ungetch();
                return make_token(tSTAR);

            case '/':
                ch = getch();
                if (ch == '=') return make_token(tSLASHEQ);
                if (ch == '*') {  // old comment
                    while (1) {
                        if (getch() != '*') continue;
                        if (getch() == '/') break;
                        ungetch();
                    }
                    continue;
                }
                if (ch == '/') {  // new comment
                    while (getch() != '\n')
                        ;
                    continue;
                }
                ungetch();
                return make_token(tSLASH);

            case '%':
                ch = getch();
                if (ch == '=') return make_token(tPERCENTEQ);
                ungetch();
                return make_token(tPERCENT);

            case '(':
                return make_token(tLPAREN);

            case ')':
                return make_token(tRPAREN);

            case '<':
                ch = getch();
                switch (ch) {
                    case '<':
                        ch = getch();
                        if (ch == '=') return make_token(tLSHIFTEQ);
                        ungetch();
                        return make_token(tLSHIFT);
                    case '=':
                        return make_token(tLTE);
                }
                ungetch();
                return make_token(tLT);

            case '>':
                ch = getch();
                switch (ch) {
                    case '>':
                        ch = getch();
                        if (ch == '=') return make_token(tRSHIFTEQ);
                        ungetch();
                        return make_token(tRSHIFT);
                    case '=':
                        return make_token(tGTE);
                }
                ungetch();
                return make_token(tGT);

            case '=':
                ch = getch();
                if (ch == '=') return make_token(tEQEQ);
                ungetch();
                return make_token(tEQ);

            case '!':
                ch = getch();
                if (ch == '=') return make_token(tNEQ);
                ungetch();
                return make_token(tEXCL);

            case '&':
                ch = getch();
                if (ch == '&') return make_token(tANDAND);
                if (ch == '=') return make_token(tANDEQ);
                ungetch();
                return make_token(tAND);

            case '^':
                ch = getch();
                if (ch == '=') return make_token(tHATEQ);
                ungetch();
                return make_token(tHAT);

            case '|':
                ch = getch();
                if (ch == '|') return make_token(tBARBAR);
                if (ch == '=') return make_token(tBAREQ);
                ungetch();
                return make_token(tBAR);

            case ';':
                return make_token(tSEMICOLON);

            case ',':
                return make_token(tCOMMA);

            case '.':
                ch = getch();
                if (ch != '.') {
                    ungetch();
                    return make_token(tDOT);
                }
                if (getch() != '.')
                    error(":%d:%d: unexpected dot", source.line, source.column);
                return make_token(tDOTS);  // ...

            case '{':
                return make_token(tLBRACE);

            case '}':
                return make_token(tRBRACE);

            case ':':
                return make_token(tCOLON);

            case '?':
                return make_token(tQUESTION);

            case '[':
                return make_token(tLBRACKET);

            case ']':
                return make_token(tRBRACKET);

            case '#':
                return make_token(tNUMBER);

            case '~':
                return make_token(tTILDE);

            case '\n':
                return make_token(tNEWLINE);
        }

        error(format("%d:%d:unexpected character", source.line, source.column));
    }

    return make_token(tEOF);
}

Vector *read_all_tokens(char *src)
{
    erase_backslash_newline(src);

    init_source(src);

    Vector *tokens = new_vector();
    while (1) {
        Token *token = read_next_token();
        vector_push_back(tokens, token);
        if (token->kind == tEOF) break;
    }

    return tokens;
}

const char *token_kind2str(int kind)
{
    switch (kind) {
        case tINT:
            return "tINT";
        case tSTRING_LITERAL:
            return "tSTRING_LITERAL";
        case tPLUS:
            return "tPLUS";
        case tMINUS:
            return "tMINUS";
        case tSTAR:
            return "tSTAR";
        case tSLASH:
            return "tSLASH";
        case tPERCENT:
            return "tPERCENT";
        case tLPAREN:
            return "tLPAREN";
        case tRPAREN:
            return "tRPAREN";
        case tLSHIFT:
            return "tLSHIFT";
        case tRSHIFT:
            return "tRSHIFT";
        case tLT:
            return "tLT";
        case tGT:
            return "tGT";
        case tLTE:
            return "tLTE";
        case tGTE:
            return "tGTE";
        case tEQEQ:
            return "tEQEQ";
        case tNEQ:
            return "tNEQ";
        case tAND:
            return "tAND";
        case tHAT:
            return "tHAT";
        case tEXCL:
            return "tEXCL";
        case tBAR:
            return "tBAR";
        case tANDAND:
            return "tANDAND";
        case tBARBAR:
            return "tBARBAR";
        case tIDENT:
            return "tIDENT";
        case tEQ:
            return "tEQ";
        case tPLUSEQ:
            return "tPLUSEQ";
        case tMINUSEQ:
            return "tMINUSEQ";
        case tSTAREQ:
            return "tSTAREQ";
        case tSLASHEQ:
            return "tSLASHEQ";
        case tPERCENTEQ:
            return "tPERCENTEQ";
        case tANDEQ:
            return "tANDEQ";
        case tHATEQ:
            return "tHATEQ";
        case tBAREQ:
            return "tBAREQ";
        case tLSHIFTEQ:
            return "tLSHIFTEQ";
        case tRSHIFTEQ:
            return "tRSHIFTEQ";
        case tSEMICOLON:
            return "tSEMICOLON";
        case tCOMMA:
            return "tCOMMA";
        case tDOT:
            return "tDOT";
        case tARROW:
            return "tARROW";
        case tLBRACE:
            return "tLBRACE";
        case tRBRACE:
            return "tRBRACE";
        case kRETURN:
            return "kRETURN";
        case tCOLON:
            return "tCOLON";
        case tQUESTION:
            return "tQUESTION";
        case tLBRACKET:
            return "tLBRACKET";
        case tRBRACKET:
            return "tRBRACKET";
        case tINC:
            return "tINC";
        case tDEC:
            return "tDEC";
        case tDOTS:
            return "tDOTS";
        case tNUMBER:
            return "tNUMBER";
        case tNEWLINE:
            return "tNEWLINE";
        case tTILDE:
            return "tTILDE";
        case tEOF:
            return "tEOF";
        case kIF:
            return "kIF";
        case kELSE:
            return "kELSE";
        case kWHILE:
            return "kWHILE";
        case kBREAK:
            return "kBREAK";
        case kCONTINUE:
            return "kCONTINUE";
        case kFOR:
            return "kFOR";
        case kINT:
            return "kINT";
        case kCHAR:
            return "kCHAR";
        case kSIZEOF:
            return "kSIZEOF";
        case kSWITCH:
            return "kSWITCH";
        case kCASE:
            return "kCASE";
        case kDEFAULT:
            return "kDEFAULT";
        case kGOTO:
            return "kGOTO";
        case kSTRUCT:
            return "kSTRUCT";
        case kUNION:
            return "kUNION";
        case kTYPEDEF:
            return "kTYPEDEF";
        case kDO:
            return "kDO";
        case kVOID:
            return "kVOID";
        case kCONST:
            return "kCONST";
        case kENUM:
            return "kENUM";
        case kNORETURN:
            return "kNORETURN";
        case kSTATIC:
            return "kSTATIC";
        case kEXTERN:
            return "kEXTERN";
        default:
            return "***unknown token***";
    }
}

Vector *concatenate_string_literal_tokens(Vector *tokens)
{
    init_tokenseq(tokens);

    Vector *ntokens = new_vector();
    while (!match_token(tEOF)) {
        if (!match_token(tSTRING_LITERAL)) {
            vector_push_back(ntokens, pop_token());
            continue;
        }

        Token *token = pop_token();
        Vector *strs = new_vector();
        vector_push_back(strs, token);
        while (match_token(tSTRING_LITERAL))
            vector_push_back(strs, pop_token());

        // calc size
        int size = 0;
        for (int i = 0; i < vector_size(strs); i++)
            size += ((Token *)vector_get(strs, i))->ssize - 1;
        size++;  // '\0'

        // concatenate strings
        char *buf = (char *)safe_malloc(size);
        int offset = 0;
        for (int i = 0; i < vector_size(strs); i++) {
            Token *token = (Token *)vector_get(strs, i);
            memcpy(buf + offset, token->sval, token->ssize - 1);
            offset += token->ssize - 1;
        }

        assert(offset == size - 1);
        buf[offset] = '\0';

        token->sval = buf;
        token->ssize = size;
        vector_push_back(ntokens, token);
    }
    vector_push_back(ntokens, pop_token());  // tEOF

    return ntokens;
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
    error("%d:%d: unexpected character: expect %c, got %c", source.line,
          source.column, expect, got);
}

char sexpect_ch(char expect)
{
    int ch = sgetch();
    if (ch != expect) unexpected_char_error(expect, ch);
    return ch;
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
            int mul = 1;
            if (peekch() == '-') {
                getch();
                mul = -1;
            }
            return new_value_code(mul * read_next_int_token()->ival);
        }

        case '(':
            return new_addrof_code(read_asm_memory(), 0);
    }

    if (isdigit(ch) || ch == '-') {
        int mul = 1;
        if (peekch() == '-') {
            getch();
            mul = -1;
        }
        int offset = mul * read_next_int_token()->ival;
        return new_addrof_code(read_asm_memory(), offset);
    }

    char *label = read_asm_token();
    return new_addrof_label_code(read_asm_memory(), label);
}

Vector *read_all_asm(char *src)
{
    init_source(src);

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
            Token *token = read_next_string_literal_token();

            Code *c = new_code(CD_ASCII);
            c->sval = token->sval;
            c->ival = token->ssize - 1;
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
        map_insert(unary_table, "pop", INST_POP);
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
            int ival = read_next_int_token()->ival;
            Code *c = new_code((int)kv_value(kv));
            c->ival = ival;
            vector_push_back(code, c);
            continue;
        }

        vector_push_back(code, new_other_code(str, NULL, NULL));
    }

    return code;
}
