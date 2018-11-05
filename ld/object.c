#include "ld.h"

void add_byte(Vector *vec, int val)
{
    vector_push_back(vec, (void *)(val & 0xff));
}

void set_byte(Vector *vec, int index, int val)
{
    vector_set(vec, index, (void *)val);
}

void add_word(Vector *vec, int val0, int val1)
{
    add_byte(vec, val0);
    add_byte(vec, val1);
}

void add_word_int(Vector *vec, int ival)
{
    add_word(vec, ival & 0xff, (ival >> 8) & 0xff);
}

void add_dword(Vector *vec, int val0, int val1, int val2, int val3)
{
    add_word(vec, val0, val1);
    add_word(vec, val2, val3);
}

void add_dword_int(Vector *vec, int ival)
{
    add_dword(vec, ival & 0xff, (ival >> 8) & 0xff, (ival >> 16) & 0xff,
              (ival >> 24) & 0xff);
}

void add_qword_int(Vector *vec, int low, int high)
{
    add_dword_int(vec, low);
    add_dword_int(vec, high);
}

void add_string(Vector *vec, char *src, int len)
{
    if (len == -1) len = strlen(src);
    for (int i = 0; i < len; i++) add_byte(vec, src[i]);
}

void add_qword(Vector *vec, int val0, int val1, int val2, int val3, int val4,
               int val5, int val6, int val7)
{
    add_dword(vec, val0, val1, val2, val3);
    add_dword(vec, val4, val5, val6, val7);
}

void write_byte(FILE *fh, int val0) { fputc(val0, fh); }

Vector *buffer_to_emit = NULL;

Vector *get_buffer_to_emit() { return buffer_to_emit; }

int emitted_size() { return vector_size(buffer_to_emit); }

void set_buffer_to_emit(Vector *buffer) { buffer_to_emit = buffer; }

void reemit_byte(int index, int val0)
{
    vector_set(buffer_to_emit, index, (void *)(val0 & 0xff));
}

void emit_byte(int val0) { add_byte(buffer_to_emit, val0); }

void emit_word(int val0, int val1)
{
    emit_byte(val0);
    emit_byte(val1);
}

void emit_word_int(int ival) { emit_word(ival & 0xff, (ival >> 8) & 0xff); }

void emit_dword(int val0, int val1, int val2, int val3)
{
    emit_word(val0, val1);
    emit_word(val2, val3);
}

void emit_dword_int(int ival)
{
    emit_dword(ival & 0xff, (ival >> 8) & 0xff, (ival >> 16) & 0xff,
               (ival >> 24) & 0xff);
}

void emit_qword(int val0, int val1, int val2, int val3, int val4, int val5,
                int val6, int val7)
{
    emit_dword(val0, val1, val2, val3);
    emit_dword(val4, val5, val6, val7);
}

void emit_qword_int(int low, int high)
{
    emit_dword_int(low);
    emit_dword_int(high);
}

void emit_string(char *src, int len) { add_string(buffer_to_emit, src, len); }

void emit_nbytes(int nbytes, int val)
{
    for (int i = 0; i < nbytes; i++) emit_byte((val >> (i << 3)) & 0xff);
}
