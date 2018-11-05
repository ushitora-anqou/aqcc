#ifndef AQCC_AQCC_H
#define AQCC_AQCC_H

//#include <assert.h>
//#include <ctype.h>
//#include <stdarg.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#ifdef __GNUC__
typedef __builtin_va_list va_list;
#else
#endif
#ifndef __GNUC__
typedef struct {
    int gp_offset;
    int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];
#endif
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

typedef struct _IO_FILE FILE;
// extern FILE *stdin;  /* Standard input stream.  */
// extern FILE *stdout; /* Standard output stream.  */
// extern FILE *stderr; /* Standard error output stream.  */
#define NULL 0
#define EOF (-1)
FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);
int fputc(int c, FILE *stream);
int fgetc(FILE *stream);
int fprintf(FILE *stream, const char *format, ...);
int printf(const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);
#define EXIT_FAILURE 1 /* Failing exit status.  */
#define EXIT_SUCCESS 0 /* Successful exit status.  */
_Noreturn void exit(int status);
void *malloc(int size);
int strlen(const char *s);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
int isalpha(int c);
int isalnum(int c);
int isdigit(int c);
int isspace(int c);
void *memcpy(void *dest, const void *src, int n);
void *memset(void *s, int c, int n);
void assert(int cond);

// vector.c
typedef struct Vector Vector;
Vector *new_vector();
Vector *new_vector_from_scalar(void *scalar);
void vector_push_back(Vector *vec, void *item);
void *vector_get(Vector *vec, int i);
int vector_size(Vector *vec);
void *vector_set(Vector *vec, int i, void *item);
void vector_push_back_vector(Vector *vec, Vector *src);
Vector *clone_vector(Vector *src);

// map.c
typedef struct KeyValue KeyValue;
typedef struct Map Map;
Map *new_map();
int map_size(Map *map);
KeyValue *map_insert(Map *map, const char *key, void *item);
KeyValue *map_lookup(Map *map, const char *key);
const char *kv_key(KeyValue *kv);
void *kv_value(KeyValue *kv);

// string_builder.c
typedef struct StringBuilder StringBuilder;
StringBuilder *new_string_builder();
char string_builder_append(StringBuilder *sb, char ch);
char *string_builder_get(StringBuilder *sb);
int string_builder_size(StringBuilder *sb);

typedef struct {
    int line, column;
    Vector *line2length;
    char *src;
    // example: "/tmp/1.c" -> cwd: "/tmp/"
    char *cwd;  // current working directory with '/'
    char *filepath;
} Source;

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
    INST_MOVL,
    INST_MOVSBL,
    INST_MOVSLQ,
    INST_MOVZB,
    INST_LEA,
    INST_PUSH,
    INST_POP,
    INST_ADD,
    INST_ADDQ,
    INST_SUB,
    INST_IMUL,
    INST_IDIV,
    INST_SAR,
    INST_SAL,
    INST_NEG,
    INST_NOT,
    INST_CMP,
    INST_SETL,
    INST_SETLE,
    INST_SETE,
    INST_AND,
    INST_XOR,
    INST_OR,
    INST_RET,
    INST_CLTD,
    INST_CLTQ,
    INST_JMP,
    INST_JE,
    INST_JNE,
    INST_JAE,
    INST_LABEL,
    INST_INCL,
    INST_INCQ,
    INST_DECL,
    INST_DECQ,
    INST_CALL,
    INST_NOP,
    INST_SYSCALL,

    CD_VALUE,
    CD_ADDR_OF,
    CD_ADDR_OF_LABEL,

    CD_GLOBAL,
    CD_TEXT,
    CD_DATA,
    CD_ZERO,
    CD_LONG,
    CD_BYTE,
    CD_QUAD,
    CD_ASCII,

    CD_COMMENT,

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
    char *sval;  // size is ival
    char *label;
    Vector *read_dep;
    int can_be_eliminated;
};

typedef struct ObjectImage ObjectImage;

// utility.c
_Noreturn void error(const char *msg, ...);
void warn(const char *msg, ...);
void *safe_malloc(int size);
char *new_str(const char *src);
int *new_int(int src);
char *format(const char *src, ...);
char *vformat(const char *src, va_list ap);
int unescape_char(int src);
char *escape_string(char *str, int size);
int min(int a, int b);
int max(int a, int b);
int roundup(int n, int b);
char *read_entire_file(char *filepath);
int is_register_code(Code *code);
int reg_of_nbyte(int nbyte, int reg);
Code *nbyte_reg(int nbyte, int reg);
Code *str2reg(char *src);
void erase_backslash_newline(char *src);

// lex.c
Vector *read_all_asm(char *src, char *filepath);
Vector *read_asm_from_filepath(char *filepath);

// code.c
Code *ADD(Code *lhs, Code *rhs);
Code *ADDQ(Code *lhs, Code *rhs);
Code *AL();
Code *AND(Code *lhs, Code *rhs);
Code *CL();
Code *CLTD();
Code *CLTQ();
Code *CMP(Code *lhs, Code *rhs);
Code *DECL(Code *lhs);
Code *DECQ(Code *lhs);
Code *IDIV(Code *lhs);
Code *IMUL(Code *lhs, Code *rhs);
Code *INCL(Code *lhs);
Code *INCQ(Code *lhs);
Code *JMP(char *label);
Code *JE(char *label);
Code *JNE(char *label);
Code *JAE(char *label);
Code *LABEL(char *label);
Code *LEA(Code *lhs, Code *rhs);
Code *MOV(Code *lhs, Code *rhs);
Code *MOVL(Code *lhs, Code *rhs);
Code *MOVSBL(Code *lhs, Code *rhs);
Code *MOVSLQ(Code *lhs, Code *rhs);
Code *MOVZB(Code *lhs, Code *rhs);
Code *NEG(Code *lhs);
Code *NOT(Code *lhs);
Code *OR(Code *lhs, Code *rhs);
Code *POP(Code *lhs);
Code *PUSH(Code *lhs);
Code *R10();
Code *R11();
Code *R12();
Code *R13();
Code *R14();
Code *R15();
Code *EAX();
Code *EDX();
Code *RAX();
Code *RBP();
Code *RDI();
Code *RDX();
Code *RET();
Code *RIP();
Code *RSP();
Code *SAL(Code *lhs, Code *rhs);
Code *SAR(Code *lhs, Code *rhs);
Code *SETE(Code *lhs);
Code *SETL(Code *lhs);
Code *SETLE(Code *lhs);
Code *SUB(Code *lhs, Code *rhs);
Code *XOR(Code *lhs, Code *rhs);
Code *GLOBAL(char *label);
Code *new_addrof_code(Code *reg, int offset);
Code *new_addrof_label_code(Code *reg, char *label);
Code *new_value_code(int value);
Code *new_code(int kind);
char *code2str(Code *code);
void dump_code(Code *code, FILE *fh);
Code *new_binop_code(int kind, Code *lhs, Code *rhs);
Code *new_unary_code(int kind, Code *lhs);

// assemble.c
ObjectImage *assemble_code(Vector *code);
void dump_object_image(ObjectImage *objimg, FILE *fh);

// object.c
void add_byte(Vector *vec, int val);
void set_byte(Vector *vec, int index, int val);
void add_word(Vector *vec, int val0, int val1);
void add_word_int(Vector *vec, int ival);
void add_dword(Vector *vec, int val0, int val1, int val2, int val3);
void add_dword_int(Vector *vec, int ival);
void add_qword_int(Vector *vec, int low, int high);
void add_string(Vector *vec, char *src, int len);
void add_qword(Vector *vec, int val0, int val1, int val2, int val3, int val4,
               int val5, int val6, int val7);
void write_byte(FILE *fh, int val0);
Vector *get_buffer_to_emit();
int emitted_size();
void set_buffer_to_emit(Vector *buffer);
void reemit_byte(int index, int val0);
void emit_byte(int val0);
void emit_word(int val0, int val1);
void emit_word_int(int ival);
void emit_dword(int val0, int val1, int val2, int val3);
void emit_dword_int(int ival);
void emit_qword(int val0, int val1, int val2, int val3, int val4, int val5,
                int val6, int val7);
void emit_qword_int(int low, int high);
void emit_string(char *src, int len);
void emit_nbytes(int nbytes, int val);

#endif
