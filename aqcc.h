#ifndef AQCC_AQCC_H
#define AQCC_AQCC_H

#include <stdlib.h>

typedef struct Vector Vector;

Vector *new_vector();
void vector_push_back(Vector *this, void *item);
void *vector_get(Vector *this, size_t i);
size_t vector_size(Vector *this);

typedef struct KeyValue KeyValue;
typedef struct Map Map;

Map *new_map();
size_t map_size(Map *map);
KeyValue *map_insert(Map *this, const char *key, void *item);
KeyValue *map_lookup(Map *this, const char *key);
const char *kv_key(KeyValue *kv);
void *kv_value(KeyValue *kv);

typedef struct {
    void *first, *second;
} Pair;

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
    tSEMICOLON,
    tCOMMA,
    tLBRACE,
    tRBRACE,
    kRETURN,
    tCOLON,
    tQUESTION,
    tINC,
    tEOF,
    kIF,
    kELSE,
    kWHILE,
    kBREAK,
    kCONTINUE,
    kFOR,
    kINT,
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

typedef struct {
    size_t idx;
} TokenSeqSaved;

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
    AST_COND,
    AST_ASSIGN,
    AST_VAR,
    AST_FUNCCALL,
    AST_FUNCDEF,
    AST_NOP,
    AST_RETURN,
    AST_EXPR_STMT,
    AST_COMPOUND,
    AST_IF,
    AST_WHILE,
    AST_BREAK,
    AST_CONTINUE,
    AST_FOR,
    AST_PREINC,
    AST_POSTINC,
    AST_ADDR,
    AST_INDIR,
};

typedef struct Env Env;
struct Env {
    Env *parent;
    Map *symbols;
    Vector *scoped_vars;
};

enum {
    TY_INT,
    TY_PTR,
};
typedef struct Type Type;
struct Type {
    int kind;
    Type *ptr_of;
};

typedef struct AST AST;
struct AST {
    int kind;
    Type *type;

    union {
        int ival;

        struct {
            char *varname;
            int stack_idx;
        };

        struct {
            AST *lhs, *rhs;
        };

        // AST_IF
        // AST_COND
        // AST_WHILE
        struct {
            AST *cond, *then, *els;
        };

        // AST_FOR
        struct {
            AST *initer, *midcond, *iterer, *for_body;
        };

        struct {
            char *fname;
            Vector *args;    // actual arguments
            Vector *params;  // formal parameters
            Type *ret_type;
            AST *body;
            Env *env;
        };

        Vector *stmts;
    };
};

// error("msg", __FILE__, __LINE__);
void error(const char *msg, const char *filename, int lineno);
void warn(const char *msg, const char *filename, int lineno);

void *safe_malloc(size_t size);
void *safe_realloc(void *ptr, size_t size);

char *new_str(const char *src);
int *new_int(int src);

AST *parse_expr(Env *env, TokenSeq *tokseq);

const char *reg_name(int byte, int i);
int type2byte(Type *type);

#endif
