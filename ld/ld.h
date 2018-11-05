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

// link.c
typedef struct ExeImage ExeImage;
ExeImage *link_objs(Vector *obj_paths);
void dump_exe_image(ExeImage *exeimg, FILE *fh);

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
