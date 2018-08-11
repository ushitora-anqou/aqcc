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

Token *read_next_int_token()
{
    int acc = 0;

    while (isdigit(peekch())) acc = 10 * acc + getch() - '0';

    Token *token = make_token(tINT);
    token->ival = acc;
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

            case '\n':
                return make_token(tNEWLINE);
        }

        error(format("%d:%d:unexpected character", source.line, source.column));
    }

    return make_token(tEOF);
}

Vector *read_all_tokens(char *src)
{
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
        case kTYPEDEF:
            return "kTYPEDEF";
        default:
            return "***unknown token***";
    }
}
