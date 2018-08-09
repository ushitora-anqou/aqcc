#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aqcc.h"

_Noreturn void error(const char *msg, const char *filename, int lineno)
{
    fprintf(stderr, "[ERROR] %s: %s, %d\n", msg, filename, lineno);
    exit(EXIT_FAILURE);
}

void warn(const char *msg, const char *filename, int lineno)
{
    fprintf(stderr, "[WARN] %s: %s, %d\n", msg, filename, lineno);
}

void *safe_malloc(size_t size)
{
    void *ptr;

    ptr = malloc(size);
    if (ptr == NULL) error("malloc failed.", __FILE__, __LINE__);
    return ptr;
}

void *safe_realloc(void *ptr, size_t size)
{
    ptr = realloc(ptr, size);
    if (ptr == NULL) error("realloc failed.", __FILE__, __LINE__);
    return ptr;
}

char *new_str(const char *src)
{
    char *ret = safe_malloc(strlen(src) + 1);
    strcpy(ret, src);
    return ret;
}

int *new_int(int src)
{
    int *ret = safe_malloc(sizeof(int));
    *ret = src;
    return ret;
}

char *format(char *src, ...)
{
    va_list args;
    char buf[512];  // TODO: enough length?

    va_start(args, src);
    vsprintf(buf, src, args);
    va_end(args);

    char *ret = safe_malloc(strlen(buf) + 1);
    strcpy(ret, buf);
    return ret;
}

int unescape_char(int src)
{
    static int table[128];
    if (table[0] == 0) {
        memset(table, 0xff, sizeof(table));

        table['n'] = '\n';
        table['r'] = '\r';
        table['t'] = '\t';
        table['0'] = '\0';
        table['a'] = '\a';
        table['b'] = '\b';
        table['v'] = '\v';
        table['f'] = '\f';
    }

    int ch = table[src];
    return ch == -1 ? src : ch;
}

char *escape_string(char *str, int size)
{
    StringBuilder *sb = new_string_builder();
    for (int i = 0; i < size; i++) {
        char ch = str[i];

        switch (ch) {
            case '\n':
                string_builder_append(sb, '\\');
                string_builder_append(sb, 'n');
                break;

            case '\r':
                string_builder_append(sb, '\\');
                string_builder_append(sb, 'n');
                break;

            case '\t':
                string_builder_append(sb, '\\');
                string_builder_append(sb, 't');
                break;

            case '\0':
                string_builder_append(sb, '\\');
                string_builder_append(sb, '0');
                break;

            case '\a':
                string_builder_append(sb, '\\');
                string_builder_append(sb, 'a');
                break;

            case '\b':
                string_builder_append(sb, '\\');
                string_builder_append(sb, 'b');
                break;

            case '\v':
                string_builder_append(sb, '\\');
                string_builder_append(sb, 'v');
                break;

            case '\f':
                string_builder_append(sb, '\\');
                string_builder_append(sb, 'f');
                break;

            case '"':
                string_builder_append(sb, '\\');
                string_builder_append(sb, '"');
                break;

            default:
                string_builder_append(sb, ch);
                break;
        }
    }

    return string_builder_get(sb);
}

char *make_label_string()
{
    static int count;
    return format(".L%d", count++);
}

int alignment_of(Type *type)
{
    switch (type->kind) {
        case TY_INT:
            return 4;
        case TY_CHAR:
            return 1;
        case TY_PTR:
            return 8;
        case TY_ARY:
            return alignment_of(type->ary_of);
        case TY_STRUCT: {
            int alignment = 0;
            for (int i = 0; i < vector_size(type->members); i++) {
                StructMember *member =
                    (StructMember *)vector_get(type->members, i);
                alignment = max(alignment, alignment_of(member->type));
            }
            return alignment;
        }
    }
    assert(0);
}

int min(int a, int b) { return a < b ? a : b; }

int max(int a, int b) { return a < b ? b : a; }

int roundup(int n, int b) { return (int)(ceil(n / (double)b)) * b; }

StructMember *lookup_member(Vector *members, char *member)
{
    // O(n)
    int size = vector_size(members);
    for (int i = 0; i < size; i++) {
        StructMember *sm = (StructMember *)vector_get(members, i);
        if (strcmp(sm->name, member) == 0) return sm;
    }
    return NULL;
}
