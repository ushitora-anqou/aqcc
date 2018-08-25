#include "aqcc.h"

void write_byte(FILE *fh, int val0) { fputc(val0, fh); }

void write_word(FILE *fh, int val0, int val1)
{
    write_byte(fh, val0);
    write_byte(fh, val1);
}

void write_dword(FILE *fh, int val0, int val1, int val2, int val3)
{
    write_word(fh, val0, val1);
    write_word(fh, val2, val3);
}

void write_qword(FILE *fh, int val0, int val1, int val2, int val3, int val4,
                 int val5, int val6, int val7)
{
    write_dword(fh, val0, val1, val2, val3);
    write_dword(fh, val4, val5, val6, val7);
}

void write_string(FILE *fh, char *src, int len)
{
    if (len == -1) len = strlen(src);
    for (int i = 0; i < len; i++) write_byte(fh, src[i]);
}

void append_byte(Vector *dumped, int val0)
{
    vector_push_back(dumped, (void *)val0);
}

void append_word(Vector *dumped, int val0, int val1)
{
    append_byte(dumped, val0);
    append_byte(dumped, val1);
}

void append_dword(Vector *dumped, int val0, int val1, int val2, int val3)
{
    append_word(dumped, val0, val1);
    append_word(dumped, val2, val3);
}

void append_dword_int(Vector *dumped, int ival)
{
    append_dword(dumped, ival & 0xff, (ival >> 8) & 0xff, (ival >> 16) & 0xff,
                 (ival >> 24) & 0xff);
}

void append_qword(Vector *dumped, int val0, int val1, int val2, int val3,
                  int val4, int val5, int val6, int val7)
{
    append_dword(dumped, val0, val1, val2, val3);
    append_dword(dumped, val4, val5, val6, val7);
}

void append_string(Vector *dumped, char *src, int len)
{
    if (len == -1) len = strlen(src);
    for (int i = 0; i < len; i++) append_byte(dumped, src[i]);
}

int reg_field(int reg)
{
    reg = reg_of_nbyte(8, reg);
    switch (reg) {
        case REG_RAX:
            return 0;
        case REG_RCX:
            return 1;
        case REG_RDX:
            return 2;
        // case REG_RBX:
        //    return 3;
        case REG_RSP:
            return 4;
        case REG_RBP:
            return 5;
        case REG_RSI:
            return 6;
        case REG_RDI:
            return 7;
        case REG_R8:
        case REG_R9:
        case REG_R10:
        case REG_R11:
        case REG_R12:
        case REG_R13:
        case REG_R14:
        case REG_R15:
            return reg - REG_R8;
    }

    assert(0);
}

int modrm(int mod, int reg, int rm)
{
    return ((mod & 3) << 6) | ((reg & 7) << 3) | (rm & 7);
}

int is_reg(int kind) { return kind & (REG_8 | REG_16 | REG_32 | REG_64); }

int is_reg64(int kind) { return is_reg(kind) && (kind & REG_64); }

int is_reg32(int kind) { return is_reg(kind) && (kind & REG_32); }

int is_imm(int kind) { return kind == CD_VALUE; }

int is_reg_ext(int kind)
{
    if (!is_reg(kind)) return 0;
    int reg = reg_of_nbyte(8, kind);
    return REG_R8 <= reg && reg <= REG_R15;
}

Vector *assemble_code_detail(Vector *code_list)
{
    Vector *dumped = new_vector();

    for (int i = 0; i < vector_size(code_list); i++) {
        Code *code = vector_get(code_list, i);

        switch (code->kind) {
            case INST_MOV:
                if (is_reg64(code->lhs->kind) && is_reg64(code->rhs->kind)) {
                    int rex_pre = 0x48;
                    if (is_reg_ext(code->lhs->kind)) rex_pre |= 4;
                    if (is_reg_ext(code->rhs->kind)) rex_pre |= 1;
                    append_byte(dumped, rex_pre);
                    append_byte(dumped, 0x89);
                    append_byte(dumped, modrm(3, reg_field(code->lhs->kind),
                                              reg_field(code->rhs->kind)));
                    break;
                }

                if (is_imm(code->lhs->kind) && is_reg32(code->rhs->kind)) {
                    if (is_reg_ext(code->rhs->kind)) append_byte(dumped, 0x41);
                    append_byte(dumped, 0xb8 + reg_field(code->rhs->kind));
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                if (is_imm(code->lhs->kind) && is_reg64(code->rhs->kind)) {
                    int rex_pre = 0x48;
                    if (is_reg_ext(code->rhs->kind)) rex_pre |= 1;
                    append_byte(dumped, rex_pre);
                    append_byte(dumped, 0xc7);
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                break;

            case INST_PUSH:
                // push %rbp
                append_byte(dumped, 0x55);
                break;

            case INST_POP:
                // pop %rbp
                append_byte(dumped, 0x5d);
                break;

            case INST_RET:
                append_byte(dumped, 0xc3);
                break;
        }
    }

    return dumped;
}

ObjectImage *assemble_code(Vector *code)
{
    ObjectImage *objimg = (ObjectImage *)safe_malloc(sizeof(ObjectImage));
    objimg->text = assemble_code_detail(code);
    return objimg;
}

void dump_object_image(ObjectImage *objimg, FILE *fh)
{
    /*
        .text
        .global main
        main:
            mov $100, %rax
            ret
        .data
    */

    //
    // *** ELF HEADRE ***
    //

    // ELF magic number
    write_dword(fh, 0x7f, 0x45, 0x4c, 0x46);
    // 64bit
    write_byte(fh, 0x02);
    // little endian
    write_byte(fh, 0x01);
    // original version of ELF
    write_byte(fh, 0x01);
    // System V
    write_byte(fh, 0x00);
    // padding
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    // ET_REL
    write_word(fh, 0x01, 0x00);
    // x86-64
    write_word(fh, 0x3e, 0x00);
    // original version of ELF
    write_dword(fh, 0x01, 0x00, 0x00, 0x00);

    // addr of entry point
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    // addr of program header table
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    // addr of section header table
    write_qword(fh, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // flag
    write_dword(fh, 0x00, 0x00, 0x00, 0x00);

    // size of this header
    write_word(fh, 0x40, 0x00);

    // size of program header table entry
    write_word(fh, 0x00, 0x00);
    // number of entries in program header table
    write_word(fh, 0x00, 0x00);

    // size of section header table entry
    write_word(fh, 0x40, 0x00);
    // number of entries in section header table
    write_word(fh, 0x07, 0x00);
    // index of section header entry containing section names
    write_word(fh, 0x06, 0x00);

    //
    // *** PROGRAM ***
    //

    // .text
    for (int i = 0; i < vector_size(objimg->text); i++)
        write_byte(fh, (int)vector_get(objimg->text, i));
    for (int i = 0; i < 8 - vector_size(objimg->text) % 8; i++)
        write_byte(fh, 0x00);

    // .data
    // .bss
    // .symtab
    write_dword(fh, 0x00, 0x00, 0x00, 0x00);  // st_name
    write_byte(fh, 0x00);                     // st_info
    write_byte(fh, 0x00);                     // st_other
    write_word(fh, 0x00, 0x00);               // st_shndx
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00);  // st_value
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);  // st_size

    write_dword(fh, 0x00, 0x00, 0x00, 0x00);  // st_name
    write_byte(fh, 0x03);                     // st_info
    write_byte(fh, 0x00);                     // st_other
    write_word(fh, 0x01, 0x00);               // st_shndx
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00);  // st_value
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);  // st_size

    write_dword(fh, 0x00, 0x00, 0x00, 0x00);  // st_name
    write_byte(fh, 0x03);                     // st_info
    write_byte(fh, 0x00);                     // st_other
    write_word(fh, 0x02, 0x00);               // st_shndx
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00);  // st_value
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);  // st_size

    write_dword(fh, 0x00, 0x00, 0x00, 0x00);  // st_name
    write_byte(fh, 0x03);                     // st_info
    write_byte(fh, 0x00);                     // st_other
    write_word(fh, 0x03, 0x00);               // st_shndx
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00);  // st_value
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);  // st_size

    write_dword(fh, 0x01, 0x00, 0x00, 0x00);  // st_name
    write_byte(fh, 0x10);                     // st_info
    write_byte(fh, 0x00);                     // st_other
    write_word(fh, 0x01, 0x00);               // st_shndx
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00);  // st_value
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);  // st_size

    // .strtab
    write_string(fh, "\0main\0", 6);

    // .strtab
    write_byte(fh, 0x00);
    write_string(fh, ".symtab\0", 8);
    write_string(fh, ".strtab\0", 8);
    write_string(fh, ".shstrtab\0", 10);
    write_string(fh, ".text\0", 6);
    write_string(fh, ".data\0", 6);
    write_string(fh, ".bss\0", 5);
    write_string(fh, "\0\0\0\0\0\0", 6);

    //
    // *** SECTION HEADER ***
    //

    // NULL
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .text
    write_qword(fh, 0x1b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00);
    write_qword(fh, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);  // offset
    write_qword(fh, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);  // size
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .data
    write_qword(fh, 0x21, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00);
    write_qword(fh, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x5e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .bss
    write_qword(fh, 0x27, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00);
    write_qword(fh, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x5e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .symtab
    write_qword(fh, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00);
    write_qword(fh, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .strtab
    write_qword(fh, 0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0xd8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .strtab
    write_qword(fh, 0x11, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0xde, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}
