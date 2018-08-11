#ifndef AQCC_AQCC_H
#define AQCC_AQCC_H

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// vector.c
typedef struct Vector Vector;
Vector *new_vector();
Vector *new_vector_from_scalar(void *scalar);
void vector_push_back(Vector *this, void *item);
void *vector_get(Vector *this, int i);
int vector_size(Vector *this);
void *vector_set(Vector *this, int i, void *item);
void vector_push_back_vector(Vector *this, Vector *src);
Vector *clone_vector(Vector *src);

// map.c
typedef struct KeyValue KeyValue;
typedef struct Map Map;
Map *new_map();
int map_size(Map *map);
KeyValue *map_insert(Map *this, const char *key, void *item);
KeyValue *map_lookup(Map *this, const char *key);
const char *kv_key(KeyValue *kv);
void *kv_value(KeyValue *kv);

// string_builder.c
typedef struct StringBuilder StringBuilder;
StringBuilder *new_string_builder();
char string_builder_append(StringBuilder *this, char ch);
char *string_builder_get(StringBuilder *this);
int string_builder_size(StringBuilder *this);

enum {
    tINT,
    tSTRING_LITERAL,
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
    tEXCL,
    tBAR,
    tANDAND,
    tBARBAR,
    tIDENT,
    tEQ,
    tPLUSEQ,
    tMINUSEQ,
    tSTAREQ,
    tSLASHEQ,
    tPERCENTEQ,
    tANDEQ,
    tHATEQ,
    tBAREQ,
    tLSHIFTEQ,
    tRSHIFTEQ,
    tSEMICOLON,
    tCOMMA,
    tDOT,
    tARROW,
    tLBRACE,
    tRBRACE,
    kRETURN,
    tCOLON,
    tQUESTION,
    tLBRACKET,
    tRBRACKET,
    tINC,
    tDEC,
    tDOTS,
    tNUMBER,
    tNEWLINE,
    tEOF,
    kIF,
    kELSE,
    kWHILE,
    kBREAK,
    kCONTINUE,
    kFOR,
    kINT,
    kCHAR,
    kSIZEOF,
    kSWITCH,
    kCASE,
    kDEFAULT,
    kGOTO,
    kSTRUCT,
    kUNION,
    kTYPEDEF,
    kDO,
    kVOID,
    kCONST,
    kENUM,
    kNORETURN,
};

typedef struct {
    int kind, line, column;

    union {
        int ival;

        struct {
            char *sval;
            int ssize;  // only when string literal. including terminating null
                        // character.
        };
    };
} Token;

typedef struct {
    Vector *tokens;
    int idx;
} TokenSeq;

typedef struct {
    int idx;
} TokenSeqSaved;

typedef struct Env Env;
struct Env {
    Env *parent;
    Map *symbols;
    Map *types;
    Vector *scoped_vars;
    Map *enum_values;
};

typedef struct AST AST;

enum {
    TY_INT,
    TY_CHAR,
    TY_PTR,
    TY_ARY,
    TY_STRUCT,
    TY_UNION,
    TY_TYPEDEF,
    TY_VOID,
    TY_ENUM,
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

        // struct/union
        struct {
            char *stname;
            Vector *members;  // for analyzer, generator
            Vector *decls;    // for parser
        };

        char *typedef_name;

        struct {
            char *enname;
            Vector *enum_list;
        };
    };
};

typedef struct GVar GVar;
struct GVar {
    char *name;
    Type *type;

    int ival;

    struct {
        char *sval;
    };
};

typedef struct {
    int cond;
    char *label_name;
} SwitchCase;

typedef struct {
    Type *type;
    char *name;
    int offset;  // when union, offset=0
} StructMember;  // also UnionMember

enum {
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_REM,
    AST_INT,
    AST_STRING_LITERAL,
    AST_UNARY_MINUS,
    AST_NOT,
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
    AST_LVAR,
    AST_GVAR,
    AST_LVAR_DECL,
    AST_GVAR_DECL,
    AST_LVAR_DECL_INIT,
    AST_GVAR_DECL_INIT,
    AST_STRUCT_VAR_DECL,
    AST_ENUM_VAR_DECL,
    AST_ENUM_VAR_DECL_INIT,
    AST_FUNCCALL,
    AST_FUNCDEF,
    AST_FUNC_DECL,
    AST_NOP,
    AST_RETURN,
    AST_EXPR_STMT,
    AST_COMPOUND,
    AST_IF,
    AST_SWITCH,
    AST_LABEL,
    AST_CASE,
    AST_DEFAULT,
    AST_WHILE,
    AST_DOWHILE,
    AST_BREAK,
    AST_CONTINUE,
    AST_FOR,
    AST_PREINC,
    AST_POSTINC,
    AST_PREDEC,
    AST_POSTDEC,
    AST_ADDR,
    AST_INDIR,
    AST_ARY2PTR,
    AST_CHAR2INT,
    AST_SIZEOF,
    AST_GOTO,
    AST_LVALUE2RVALUE,
    AST_MEMBER_REF,
    AST_MEMBER_REF_PTR,
    AST_EXPR_LIST,
    AST_DECL_LIST,
    AST_TYPEDEF_VAR_DECL,
    AST_CAST,
};

struct AST {
    int kind;
    Type *type;

    union {
        int ival;

        struct {
            char *sval;
            int ssize;  // only when string literal. including null character.
        };

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
            AST *body;       // If NULL then only declaration exists.
            Env *env;
        };

        // AST_ARY2PTR
        struct {
            AST *ary;
        };

        struct {
            char *label_name;
            AST *label_stmt;
        };

        struct {
            AST *target, *switch_body;
            Vector *cases;
            char *default_label;
        };

        struct {
            AST *stsrc;
            char *member;
        };

        Vector *stmts;
        Vector *exprs;
        Vector *decls;
    };
};

// utility.c
_Noreturn void error(const char *msg, ...);
_Noreturn void error_unexpected_token_kind(int expect_kind, Token *got);
_Noreturn void error_unexpected_token_str(char *expect_str, Token *got);
void warn(const char *msg, ...);
void *safe_malloc(int size);
void *safe_realloc(void *ptr, int size);
char *new_str(const char *src);
int *new_int(int src);
char *format(const char *src, ...);
char *vformat(const char *src, va_list ap);
int unescape_char(int src);
char *escape_string(char *str, int size);
char *make_label_string();
int alignment_of(Type *type);
int min(int a, int b);
int max(int a, int b);
int roundup(int n, int b);

// lex.c
Vector *read_all_tokens(char *src);
const char *token_kind2str(int kind);

// parse.c
Vector *parse_prog(Vector *tokens);

// gen.c
Vector *generate_code(Vector *asts);
void dump_code(Vector *codes, FILE *fh);

// type.c
Type *type_int();
Type *type_char();
Type *type_void();
Type *new_pointer_type(Type *src);
Type *new_array_type(Type *src, int len);
Env *new_env(Env *parent);
Type *new_struct_or_union_type(int kind, char *stname, Vector *members);
Type *type_unknown();
Type *new_typedef_type(char *typedef_name);
Type *new_enum_type(char *name, Vector *list);

// env.c
AST *add_var(Env *env, AST *ast);
AST *lookup_var(Env *env, const char *name);
AST *add_func(Env *env, const char *name, AST *ast);
AST *lookup_func(Env *env, const char *name);
Type *add_type(Env *env, Type *type, char *name);
Type *lookup_type(Env *env, const char *name);
Type *add_struct_or_union_or_enum_type(Env *env, Type *type);
Type *lookup_struct_or_union_or_enum_type(Env *env, const char *name);
int add_enum_value(Env *env, char *name, int ival);
int *lookup_enum_value(Env *env, char *name);

// ast.c
int match_type(AST *ast, int kind);
int match_type2(AST *lhs, AST *rhs, int lkind, int rkind);
AST *new_ast(int kind);
AST *new_binop_ast(int kind, AST *lhs, AST *rhs);
AST *new_while_stmt(AST *cond, AST *body);
AST *new_compound_stmt2(AST *first, AST *second);
AST *new_ary2ptr_ast(AST *ary);
AST *ary2ptr(AST *ary);
AST *char2int(AST *ch);
AST *new_var_ast(char *varname);
AST *new_var_decl_ast(int kind, Type *type, char *varname);
AST *new_var_decl_init_ast(AST *var_decl, AST *initer);
AST *new_unary_ast(int kind, AST *that);
AST *new_func_ast(int kind, char *fname, Vector *args, Vector *params,
                  Type *ret_type);
AST *new_label_ast(char *name, AST *stmt);
AST *new_lvalue2rvalue_ast(AST *lvalue);
AST *new_int_ast(int ival);

// analyze.c
void analyze_ast(Vector *asts);
Vector *get_gvar_list();

// cpp.c
Vector *preprocess_tokens(Vector *tokens);

// token.c
Token *new_token(int kind, int line, int column);
TokenSeq *new_token_seq(Vector *tokens);
void init_tokenseq(Vector *tokens);
Token *peek_token();
Token *pop_token();
Token *expect_token(int kind);
int match_token(int kind);
Token *pop_token_if(int kind);
int match_token2(int kind0, int kind1);
TokenSeqSaved *new_token_seq_saved();
void restore_token_seq_saved(TokenSeqSaved *saved);

#define SAVE_TOKENSEQ \
    TokenSeqSaved *token_seq_saved__dummy = new_token_seq_saved();
#define RESTORE_TOKENSEQ restore_token_seq_saved(token_seq_saved__dummy);

#endif
