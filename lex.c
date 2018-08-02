#include "aqcc.h"

Token *new_token(int kind)
{
    Token *token = safe_malloc(sizeof(Token));
    token->kind = kind;
    return token;
}

Token *read_next_int_token(FILE *fh)
{
    char buf[256];  // TODO: enough length?
    int bufidx = 0;

    while (1) {
        int ch = fgetc(fh);

        if (!isdigit(ch)) {
            ungetc(ch, fh);
            break;
        }

        buf[bufidx++] = ch;
    }

    Token *token = new_token(tINT);
    buf[bufidx++] = '\0';
    token->ival = atoi(buf);
    return token;
}

Token *read_next_ident_token(FILE *fh)
{
    StringBuilder *sb;
    char *str;

    sb = new_string_builder();

    string_builder_append(sb, fgetc(fh));

    while (1) {
        int ch = fgetc(fh);

        if (!isalnum(ch) && ch != '_') {
            ungetc(ch, fh);
            break;
        }

        string_builder_append(sb, ch);
    }

    str = string_builder_get(sb);

    if (strcmp(str, "return") == 0) return new_token(kRETURN);
    if (strcmp(str, "if") == 0) return new_token(kIF);
    if (strcmp(str, "else") == 0) return new_token(kELSE);
    if (strcmp(str, "while") == 0) return new_token(kWHILE);
    if (strcmp(str, "break") == 0) return new_token(kBREAK);
    if (strcmp(str, "continue") == 0) return new_token(kCONTINUE);
    if (strcmp(str, "for") == 0) return new_token(kFOR);
    if (strcmp(str, "int") == 0) return new_token(kINT);
    if (strcmp(str, "char") == 0) return new_token(kCHAR);
    if (strcmp(str, "sizeof") == 0) return new_token(kSIZEOF);

    Token *token = new_token(tIDENT);
    token->sval = str;
    return token;
}

// assume that the first doublequote has been already read.
Token *read_next_string_literal_token(FILE *fh)
{
    StringBuilder *sb;
    Token *token;

    sb = new_string_builder();

    while (1) {
        int ch;

        ch = fgetc(fh);
        if (ch == '"') break;
        if (ch == EOF) error("unexpected EOF", __FILE__, __LINE__);
        if (ch == '\n')
            error("unexpected new-line character", __FILE__, __LINE__);

        string_builder_append(sb, ch);
    }

    token = new_token(tSTRING_LITERAL);
    token->sval = string_builder_get(sb);
    return token;
}

Token *read_next_token(FILE *fh)
{
    while (1) {
        int ch;

        ch = fgetc(fh);

        if (isspace(ch)) continue;

        if (isdigit(ch)) {
            ungetc(ch, fh);
            return read_next_int_token(fh);
        }

        if (isalpha(ch) || ch == '_') {
            ungetc(ch, fh);
            return read_next_ident_token(fh);
        }

        switch (ch) {
            case '"':
                return read_next_string_literal_token(fh);

            case '+':
                ch = fgetc(fh);
                if (ch == '+') return new_token(tINC);
                if (ch == '=') return new_token(tPLUSEQ);
                ungetc(ch, fh);
                return new_token(tPLUS);
            case '-':
                ch = fgetc(fh);
                if (ch == '=') return new_token(tMINUSEQ);
                ungetc(ch, fh);
                return new_token(tMINUS);
            case '*':
                ch = fgetc(fh);
                if (ch == '=') return new_token(tSTAREQ);
                ungetc(ch, fh);
                return new_token(tSTAR);
            case '/':
                ch = fgetc(fh);
                if (ch == '=') return new_token(tSLASHEQ);
                ungetc(ch, fh);
                return new_token(tSLASH);
            case '%':
                ch = fgetc(fh);
                if (ch == '=') return new_token(tPERCENTEQ);
                ungetc(ch, fh);
                return new_token(tPERCENT);
            case '(':
                return new_token(tLPAREN);
            case ')':
                return new_token(tRPAREN);
            case '<':
                ch = fgetc(fh);
                switch (ch) {
                    case '<':
                        ch = fgetc(fh);
                        if (ch == '=') return new_token(tLSHIFTEQ);
                        ungetc(ch, fh);
                        return new_token(tLSHIFT);
                    case '=':
                        return new_token(tLTE);
                }
                ungetc(ch, fh);
                return new_token(tLT);
            case '>':
                ch = fgetc(fh);
                switch (ch) {
                    case '>':
                        ch = fgetc(fh);
                        if (ch == '=') return new_token(tRSHIFTEQ);
                        ungetc(ch, fh);
                        return new_token(tRSHIFT);
                    case '=':
                        return new_token(tGTE);
                }
                ungetc(ch, fh);
                return new_token(tGT);
            case '=':
                ch = fgetc(fh);
                if (ch == '=') return new_token(tEQEQ);
                ungetc(ch, fh);
                return new_token(tEQ);
            case '!':
                ch = fgetc(fh);
                if (ch != '=') break;
                return new_token(tNEQ);
            case '&':
                ch = fgetc(fh);
                if (ch == '&') return new_token(tANDAND);
                if (ch == '=') return new_token(tANDEQ);
                ungetc(ch, fh);
                return new_token(tAND);
            case '^':
                ch = fgetc(fh);
                if (ch == '=') return new_token(tHATEQ);
                ungetc(ch, fh);
                return new_token(tHAT);
            case '|':
                ch = fgetc(fh);
                if (ch == '|') return new_token(tBARBAR);
                if (ch == '=') return new_token(tBAREQ);
                ungetc(ch, fh);
                return new_token(tBAR);
            case ';':
                return new_token(tSEMICOLON);
            case ',':
                return new_token(tCOMMA);
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
            case EOF:
                return new_token(tEOF);
        }

        error("unexpected token", __FILE__, __LINE__);
    }
}

Vector *read_all_tokens(FILE *fh)
{
    Vector *tokens = new_vector();

    while (1) {
        Token *token = read_next_token(fh);
        vector_push_back(tokens, token);
        if (token->kind == tEOF) break;
    }

    return tokens;
}
