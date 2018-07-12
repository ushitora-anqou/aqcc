#pragma once
#ifndef AQCC_AQCC_H

#include <stdlib.h>

typedef struct {
    size_t size, rsved_size;
    void **data;
} Vector;

Vector *new_vector();
void vector_push_back(Vector *this, void *item);
void *vector_get(Vector *this, size_t i);
size_t vector_size(Vector *this);

typedef struct {
    const char *key;
    void *value;
} KeyValue;

typedef struct {
    Vector *data;
} Map;

Map *new_map();
size_t map_size(Map *map);
KeyValue *map_insert(Map *this, const char *key, void *item);
KeyValue *map_lookup(Map *this, const char *key);

enum {
    tINT,
    tPLUS,
    tMINUS,
    tSTAR,
    tSLASH,
    tPERCENT,
    tLPAREN,
    tRPAREN,
    tLSHIFT,
    tRSHIFT,
    tLT,
    tGT,
    tLTE,
    tGTE,
    tEQEQ,
    tNEQ,
    tAND,
    tHAT,
    tBAR,
    tANDAND,
    tBARBAR,
    tIDENT,
    tEQ,
    tEOF,
    tSEMICOLON,
};

typedef struct {
    int kind;

    union {
        int ival;
        char *sval;
    };
} Token;

typedef struct {
    Vector *tokens;
    size_t idx;
} TokenSeq;

enum {
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_REM,
    AST_INT,
    AST_UNARY_MINUS,
    AST_LSHIFT,
    AST_RSHIFT,
    AST_LT,
    AST_GT,
    AST_LTE,
    AST_GTE,
    AST_EQ,
    AST_NEQ,
    AST_AND,
    AST_XOR,
    AST_OR,
    AST_LAND,
    AST_LOR,
    AST_ASSIGN,
    AST_VAR,
    AST_FUNCCALL,
    AST_NOP,
};

typedef struct AST AST;
struct AST {
    int kind;

    union {
        int ival;
        char *sval;

        struct {
            AST *lhs, *rhs;
        };

        struct {
            char *fname;
        };
    };
};

// error("msg", __FILE__, __LINE__);
void error(const char *msg, const char *filename, int lineno);

void *safe_malloc(size_t size);
void *safe_realloc(void *ptr, size_t size);

char *new_str(const char *src);
int *new_int(int src);

AST *parse_expr(TokenSeq *tokseq);

#endif
