#include "aqcc.h"

typedef struct {
    int line, column;
    char *src;
} SrcFile;

char getch(SrcFile *file) { return *file->src++; }

void ungetch(SrcFile *file) { file->src--; }

char peekch(SrcFile *file) { return *file->src; }

Token *new_token(int kind)
{
    Token *token = safe_malloc(sizeof(Token));
    token->kind = kind;
    return token;
}

Token *read_next_int_token(SrcFile *file)
{
    int acc = 0;

    while (isdigit(peekch(file))) acc = 10 * acc + getch(file) - '0';

    Token *token = new_token(tINT);
    token->ival = acc;
    return token;
}

Token *read_next_ident_token(SrcFile *file)
{
    StringBuilder *sb = new_string_builder();
    while (1) {
        int ch = getch(file);

        if (!isalnum(ch) && ch != '_') {
            ungetch(file);
            break;
        }

        string_builder_append(sb, ch);
    }

    char *str;
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
Token *read_next_string_literal_token(SrcFile *file)
{
    StringBuilder *sb = new_string_builder();
    while (1) {
        char ch = getch(file);
        if (ch == '"') break;
        if (ch == '\0') error("unexpected EOF", __FILE__, __LINE__);
        if (ch == '\n')
            error("unexpected new-line character", __FILE__, __LINE__);

        string_builder_append(sb, ch);
    }

    Token *token;
    token = new_token(tSTRING_LITERAL);
    token->sval = string_builder_get(sb);
    return token;
}

Token *read_next_token(SrcFile *file)
{
    while (peekch(file) != '\0') {
        char ch = getch(file);

        if (isspace(ch)) continue;

        if (isdigit(ch)) {
            ungetch(file);
            return read_next_int_token(file);
        }

        if (isalpha(ch) || ch == '_') {
            ungetch(file);
            return read_next_ident_token(file);
        }

        switch (ch) {
            case '"':
                return read_next_string_literal_token(file);

            case '+':
                ch = getch(file);
                if (ch == '+') return new_token(tINC);
                if (ch == '=') return new_token(tPLUSEQ);
                ungetch(file);
                return new_token(tPLUS);

            case '-':
                ch = getch(file);
                if (ch == '=') return new_token(tMINUSEQ);
                ungetch(file);
                return new_token(tMINUS);

            case '*':
                ch = getch(file);
                if (ch == '=') return new_token(tSTAREQ);
                ungetch(file);
                return new_token(tSTAR);

            case '/':
                ch = getch(file);
                if (ch == '=') return new_token(tSLASHEQ);
                ungetch(file);
                return new_token(tSLASH);

            case '%':
                ch = getch(file);
                if (ch == '=') return new_token(tPERCENTEQ);
                ungetch(file);
                return new_token(tPERCENT);

            case '(':
                return new_token(tLPAREN);

            case ')':
                return new_token(tRPAREN);

            case '<':
                ch = getch(file);
                switch (ch) {
                    case '<':
                        ch = getch(file);
                        if (ch == '=') return new_token(tLSHIFTEQ);
                        ungetch(file);
                        return new_token(tLSHIFT);
                    case '=':
                        return new_token(tLTE);
                }
                ungetch(file);
                return new_token(tLT);

            case '>':
                ch = getch(file);
                switch (ch) {
                    case '>':
                        ch = getch(file);
                        if (ch == '=') return new_token(tRSHIFTEQ);
                        ungetch(file);
                        return new_token(tRSHIFT);
                    case '=':
                        return new_token(tGTE);
                }
                ungetch(file);
                return new_token(tGT);

            case '=':
                ch = getch(file);
                if (ch == '=') return new_token(tEQEQ);
                ungetch(file);
                return new_token(tEQ);

            case '!':
                ch = getch(file);
                if (ch != '=') break;
                return new_token(tNEQ);

            case '&':
                ch = getch(file);
                if (ch == '&') return new_token(tANDAND);
                if (ch == '=') return new_token(tANDEQ);
                ungetch(file);
                return new_token(tAND);

            case '^':
                ch = getch(file);
                if (ch == '=') return new_token(tHATEQ);
                ungetch(file);
                return new_token(tHAT);

            case '|':
                ch = getch(file);
                if (ch == '|') return new_token(tBARBAR);
                if (ch == '=') return new_token(tBAREQ);
                ungetch(file);
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
        }

        error("unexpected token", __FILE__, __LINE__);
    }

    return new_token(tEOF);
}

Vector *read_all_tokens(FILE *fh)
{
    // read the file all
    StringBuilder *sb = new_string_builder();
    int ch;
    while ((ch = fgetc(fh)) != EOF) string_builder_append(sb, ch);
    SrcFile file;
    file.src = string_builder_get(sb);

    Vector *tokens = new_vector();
    while (1) {
        Token *token = read_next_token(&file);
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
        default:
            return "***unknown token***";
    }
}
