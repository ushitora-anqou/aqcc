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

Token *new_token(int kind)
{
    Token *token = safe_malloc(sizeof(Token));
    token->kind = kind;
    token->line = source.line;
    token->column = source.column;
    return token;
}

Token *read_next_int_token()
{
    int acc = 0;

    while (isdigit(peekch())) acc = 10 * acc + getch() - '0';

    Token *token = new_token(tINT);
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
    }

    char *str;
    str = string_builder_get(sb);
    KeyValue *kv = map_lookup(str2keyword, str);
    if (kv) return new_token((int)kv_value(kv));

    Token *token = new_token(tIDENT);
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
    token = new_token(tSTRING_LITERAL);
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

    Token *token = new_token(tINT);
    token->ival = ch;
    return token;
}

Token *read_next_token()
{
    while (peekch() != '\0') {
        char ch = getch();

        if (isspace(ch)) continue;

        if (isdigit(ch)) {
            ungetch();
            return read_next_int_token();
        }

        if (isalpha(ch) || ch == '_') {
            ungetch();
            return read_next_ident_token();
        }

        switch (ch) {
            case '"':
                return read_next_string_literal_token();

            case '\'':
                return read_next_character_constant_token();

            case '+':
                ch = getch();
                if (ch == '+') return new_token(tINC);
                if (ch == '=') return new_token(tPLUSEQ);
                ungetch();
                return new_token(tPLUS);

            case '-':
                ch = getch();
                if (ch == '=') return new_token(tMINUSEQ);
                if (ch == '>') return new_token(tARROW);
                if (ch == '-') return new_token(tDEC);
                ungetch();
                return new_token(tMINUS);

            case '*':
                ch = getch();
                if (ch == '=') return new_token(tSTAREQ);
                ungetch();
                return new_token(tSTAR);

            case '/':
                ch = getch();
                if (ch == '=') return new_token(tSLASHEQ);
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
                return new_token(tSLASH);

            case '%':
                ch = getch();
                if (ch == '=') return new_token(tPERCENTEQ);
                ungetch();
                return new_token(tPERCENT);

            case '(':
                return new_token(tLPAREN);

            case ')':
                return new_token(tRPAREN);

            case '<':
                ch = getch();
                switch (ch) {
                    case '<':
                        ch = getch();
                        if (ch == '=') return new_token(tLSHIFTEQ);
                        ungetch();
                        return new_token(tLSHIFT);
                    case '=':
                        return new_token(tLTE);
                }
                ungetch();
                return new_token(tLT);

            case '>':
                ch = getch();
                switch (ch) {
                    case '>':
                        ch = getch();
                        if (ch == '=') return new_token(tRSHIFTEQ);
                        ungetch();
                        return new_token(tRSHIFT);
                    case '=':
                        return new_token(tGTE);
                }
                ungetch();
                return new_token(tGT);

            case '=':
                ch = getch();
                if (ch == '=') return new_token(tEQEQ);
                ungetch();
                return new_token(tEQ);

            case '!':
                ch = getch();
                if (ch == '=') return new_token(tNEQ);
                ungetch();
                return new_token(tEXCL);

            case '&':
                ch = getch();
                if (ch == '&') return new_token(tANDAND);
                if (ch == '=') return new_token(tANDEQ);
                ungetch();
                return new_token(tAND);

            case '^':
                ch = getch();
                if (ch == '=') return new_token(tHATEQ);
                ungetch();
                return new_token(tHAT);

            case '|':
                ch = getch();
                if (ch == '|') return new_token(tBARBAR);
                if (ch == '=') return new_token(tBAREQ);
                ungetch();
                return new_token(tBAR);

            case ';':
                return new_token(tSEMICOLON);

            case ',':
                return new_token(tCOMMA);

            case '.':
                return new_token(tDOT);

            case '{':
                return new_token(tLBRACE);

            case '}':
                return new_token(tRBRACE);

            case ':':
                return new_token(tCOLON);

            case '?':
                return new_token(tQUESTION);

            case '[':
                return new_token(tLBRACKET);

            case ']':
                return new_token(tRBRACKET);
        }

        error(format("%d:%d:unexpected character", source.line, source.column));
    }

    return new_token(tEOF);
}

Vector *read_all_tokens(FILE *fh)
{
    // read the file all
    StringBuilder *sb = new_string_builder();
    int ch;
    while ((ch = fgetc(fh)) != EOF) string_builder_append(sb, ch);
    init_source(string_builder_get(sb));

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
