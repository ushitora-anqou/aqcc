#ifndef AQCC_AQCC_H
#define AQCC_AQCC_H

//#include <assert.h>
//#include <ctype.h>
//#include <stdarg.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

typedef struct {
    int gp_offset;
    int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];

typedef struct _IO_FILE FILE;
extern FILE *stdin;  /* Standard input stream.  */
extern FILE *stdout; /* Standard output stream.  */
extern FILE *stderr; /* Standard error output stream.  */
#define NULL 0
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
#define EXIT_FAILURE 1 /* Failing exit status.  */
#define EXIT_SUCCESS 0 /* Successful exit status.  */
void exit(int status);
void *malloc(int size);
void *realloc(void *ptr, int size);
int strlen(const char *s);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
int vsprintf(char *str, const char *format, va_list ap);
void *memset(void *s, int c, int n);
int fputc(int c, FILE *stream);
int fgetc(FILE *stream);
#define EOF (-1)
FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);
int isalnum(int c);
int isalpha(int c);
int isdigit(int c);
int isspace(int c);
void *memcpy(void *dest, const void *src, int n);

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
    tTILDE,
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
    kSTATIC,
    kEXTERN,
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
    int kind, nbytes, is_static, is_extern;

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
    char *sval;
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
    AST_COMPL,
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
    AST_VA_START,
    AST_VA_END,
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
            char *varname, *gen_varname;
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
            int is_variadic;
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

enum {
    REG_8 = 1 << 5,
    REG_AL = 0 | REG_8,
    REG_DIL,
    REG_SIL,
    REG_DL,
    REG_CL,
    REG_R8B,
    REG_R9B,
    REG_R10B,
    REG_R11B,
    REG_R12B,
    REG_R13B,
    REG_R14B,
    REG_R15B,
    REG_BPL,
    REG_SPL,

    REG_16 = 1 << 6,
    REG_AX = 0 | REG_16,
    REG_DI,
    REG_SI,
    REG_DX,
    REG_CX,
    REG_R8W,
    REG_R9W,
    REG_R10W,
    REG_R11W,
    REG_R12W,
    REG_R13W,
    REG_R14W,
    REG_R15W,
    REG_BP,
    REG_SP,

    REG_32 = 1 << 7,
    REG_EAX = 0 | REG_32,
    REG_EDI,
    REG_ESI,
    REG_EDX,
    REG_ECX,
    REG_R8D,
    REG_R9D,
    REG_R10D,
    REG_R11D,
    REG_R12D,
    REG_R13D,
    REG_R14D,
    REG_R15D,
    REG_EBP,
    REG_ESP,

    REG_64 = 1 << 8,
    REG_RAX = 0 | REG_64,
    REG_RDI,
    REG_RSI,
    REG_RDX,
    REG_RCX,
    REG_R8,
    REG_R9,
    REG_R10,
    REG_R11,
    REG_R12,
    REG_R13,
    REG_R14,
    REG_R15,
    REG_RBP,
    REG_RSP,

    REG_RIP,

    INST_ = 1 << 9,
    INST_MOV = 0 | INST_,
    INST_MOVSBL,
    INST_MOVZB,
    INST_LEA,
    INST_PUSH,
    INST_POP,
    INST_ADD,
    INST_SUB,
    INST_IMUL,
    INST_IDIV,
    INST_SAR,
    INST_SAL,
    INST_NEG,
    INST_COMPL,
    INST_CMP,
    INST_SETL,
    INST_SETLE,
    INST_SETE,
    INST_AND,
    INST_XOR,
    INST_OR,
    INST_RET,

    INST_OTHER,

    CD_VALUE,
    CD_ADDR_OF,
    CD_ADDR_OF_LABEL,

    MRK_BASIC_BLOCK_START,
    MRK_BASIC_BLOCK_END,
    MRK_FUNCDEF_START,
    MRK_FUNCDEF_END,
    MRK_FUNCDEF_RETURN,
};

typedef struct Code Code;
struct Code {
    int kind;

    Code *lhs, *rhs;
    int ival;
    char *label;
    char *other_op;
    Vector *read_dep;
    int can_be_eliminated;
};

typedef struct ObjectImage ObjectImage;
struct ObjectImage {
    Vector *text;
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
Vector *read_tokens_from_filepath(char *filepath);
void assert(int cond);
int is_register_code(Code *code);
int reg_of_nbyte(int nbyte, int reg);

// lex.c
Vector *read_all_tokens(char *src);
const char *token_kind2str(int kind);
Vector *concatenate_string_literal_tokens(Vector *tokens);

// parse.c
Vector *parse_prog(Vector *tokens);

// gen.c
void dump_code(Code *code, FILE *fh);
Vector *generate_register_code(Vector *asts);
Code *new_code(int kind);
Code *PUSH(Code *lhs);
Code *POP(Code *lhs);
Code *R12();
Code *R13();
Code *R14();
Code *R15();

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
Type *new_static_type(Type *type);
Type *new_extern_type(Type *type);
void move_static_extern_specifier(Type *src, Type *dst);

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
AST *new_lgvar_ast(int kind, Type *type, char *varname, int stack_idx);
AST *new_var_decl_ast(int kind, Type *type, char *varname);
AST *new_var_decl_init_ast(AST *var_decl, AST *initer);
AST *new_unary_ast(int kind, AST *that);
AST *new_func_ast(int kind, char *fname, Vector *args, Vector *params,
                  Type *ret_type);
AST *new_label_ast(char *name, AST *stmt);
AST *new_lvalue2rvalue_ast(AST *lvalue);
AST *new_int_ast(int ival);

// analyze.c
Env *analyze_ast(Vector *asts);
Vector *get_gvar_list();

// cpp.c
Vector *preprocess_tokens(Vector *tokens);

// token.c
Token *new_token(int kind, int line, int column);
TokenSeq *new_token_seq(Vector *tokens);
void init_tokenseq(Vector *tokens);
void insert_tokens(Vector *tokens);
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

// optimize.c
AST *optimize_ast_constant(AST *ast, Env *env);
void optimize_asts_constant(Vector *asts, Env *env);
Vector *optimize_code(Vector *code);

// assemble.c
ObjectImage *assemble_code(Vector *code);
void dump_object_image(ObjectImage *objimg, FILE *fh);

#endif
