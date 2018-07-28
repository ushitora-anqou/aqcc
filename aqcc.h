#ifndef AQCC_AQCC_H
#define AQCC_AQCC_H

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    tLBRACKET,
    tRBRACKET,
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
    AST_LVAR,
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
    AST_ARY2PTR,
    AST_GVAR,
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
    TY_ARY,
};
typedef struct Type Type;
struct Type {
    int kind, nbytes;

    union {
        Type *ptr_of;

        struct {
            Type *ary_of;
            int len;
        };
    };
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

        // AST_ARY2PTR
        struct {
            AST *ary;
        };
    };
};

// utility.c
// error("msg", __FILE__, __LINE__);
void error(const char *msg, const char *filename, int lineno);
void warn(const char *msg, const char *filename, int lineno);
void *safe_malloc(size_t size);
void *safe_realloc(void *ptr, size_t size);
char *new_str(const char *src);
int *new_int(int src);
const char *reg_name(int byte, int i);
Pair *new_pair(void *first, void *second);
int max(int a, int b);

// lex.c
Vector *read_all_tokens(FILE *fh);

// parse.c
Vector *parse_prog(Vector *tokens);

// gen.c
Vector *generate_code(Vector *asts);
void dump_codes(Vector *codes, FILE *fh);

// type.c
Type *type_int();
Type *new_pointer_type(Type *src);
Type *new_array_type(Type *src, int len);
Env *new_env(Env *parent);

// env.c
AST *add_var(Env *env, const char *name, AST *ast);
AST *lookup_var(Env *env, const char *name);
AST *add_func(Env *env, const char *name, AST *ast);
AST *lookup_func(Env *env, const char *name);

// ast.c
int match_type(AST *ast, int kind);
int match_type2(AST *lhs, AST *rhs, int lkind, int rkind);
AST *new_ast(int kind);
AST *new_binop_ast(int kind, AST *lhs, AST *rhs, Type *type);
AST *new_funccall_ast(char *fname, Vector *args, Type *ret_type);
AST *new_funcdef_ast(char *fname, Vector *params, Type *ret_type);
AST *new_expr_stmt(AST *expr);
AST *new_while_stmt(AST *cond, AST *body);
AST *new_compound_stmt2(AST *first, AST *second);
AST *new_ary2ptr_ast(AST *ary);
AST *new_indir_ast(AST *src);
AST *ary2ptr(AST *ary);
AST *new_add_ast(AST *lhs, AST *rhs);

#endif
