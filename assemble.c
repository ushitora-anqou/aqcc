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

void write_dword_int(FILE *fh, int ival)
{
    write_dword(fh, ival & 0xff, (ival >> 8) & 0xff, (ival >> 16) & 0xff,
                (ival >> 24) & 0xff);
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

int is_reg(Code *code) { return is_register_code(code); }

int is_reg64(Code *code) { return is_reg(code) && (code->kind & REG_64); }

int is_reg32(Code *code) { return is_reg(code) && (code->kind & REG_32); }

int is_reg8(Code *code) { return is_reg(code) && (code->kind & REG_8); }

int is_imm(Code *code) { return code->kind == CD_VALUE; }

int is_reg_ext(Code *code)
{
    if (!is_reg(code)) return 0;
    int reg = reg_of_nbyte(8, code->kind);
    return REG_R8 <= reg && reg <= REG_R15;
}

int is_addrof(Code *code) { return code->kind == CD_ADDR_OF; }

int reg_field(Code *code)
{
    assert(is_reg(code));

    int reg = reg_of_nbyte(8, code->kind);
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

int append_modrm(Vector *dumped, int mod, int reg, int rm)
{
    append_byte(dumped, modrm(mod, reg, rm));

    // If mod == 2 and rm == 4 then SIB byte is enabled.
    // If I == 0 in SIB then index is disabled.
    // That's why this line is needed.
    // NOT CARGO CULT PROGRAMMING!!
    // But this spec is awful, isn't it :thinking_face:
    if (mod == 2 && rm == 4) append_byte(dumped, modrm(0, 4, 4));
}

int rex_prefix(int w, int r, int x, int b)
{
    // assume that w, r, x, b are either 0 or 1.
    return 0x40 | (w << 3) | (r << 2) | (x << 1) | b;
}

int rex_prefix_reg_ext(int is64, Code *reg, Code *rm)
{
    return rex_prefix(is64, is_reg_ext(reg), 0, is_reg_ext(rm));
}

void append_rex_prefix(Vector *dumped, int is64, Code *reg, Code *rm)
{
    int pre = rex_prefix_reg_ext(is64, reg, rm);
    if (pre == 0x40) return;  // no info
    append_byte(dumped, pre);
}

void append_addrof(Vector *dumped, int reg, Code *mem)
{
    assert(is_addrof(mem));
    append_modrm(dumped, 2, reg, reg_field(mem->lhs));
    append_dword_int(dumped, mem->ival);
}

ObjectImage *assemble_code_detail(Vector *code_list)
{
    Vector *dumped = new_vector();
    Map *label2offset = new_map();
    Vector *label_placeholders = new_vector();
    typedef struct {
        char *label;
        int offset, size;
    } LabelPlaceholder;

    for (int i = 0; i < vector_size(code_list); i++) {
        Code *code = vector_get(code_list, i);

        switch (code->kind) {
            case INST_MOV:
                if (is_reg64(code->lhs) && is_reg64(code->rhs)) {
                    append_byte(dumped,
                                rex_prefix_reg_ext(1, code->lhs, code->rhs));
                    append_byte(dumped, 0x89);
                    append_byte(dumped, modrm(3, reg_field(code->lhs),
                                              reg_field(code->rhs)));
                    break;
                }

                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    append_rex_prefix(dumped, 0, code->lhs, code->rhs);
                    append_byte(dumped, 0x89);
                    append_byte(dumped, modrm(3, reg_field(code->lhs),
                                              reg_field(code->rhs)));
                    break;
                }

                if (is_reg8(code->lhs) && is_reg8(code->rhs)) {
                    append_rex_prefix(dumped, 0, code->lhs, code->rhs);
                    append_byte(dumped, 0x88);
                    append_byte(dumped, modrm(3, reg_field(code->lhs),
                                              reg_field(code->rhs)));
                    break;
                }

                if (is_imm(code->lhs) && is_reg32(code->rhs)) {
                    if (is_reg_ext(code->rhs)) append_byte(dumped, 0x41);
                    append_byte(dumped, 0xb8 + reg_field(code->rhs));
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                if (is_imm(code->lhs) && is_reg64(code->rhs)) {
                    append_byte(dumped, rex_prefix_reg_ext(1, NULL, code->rhs));
                    append_byte(dumped, 0xc7);
                    append_byte(dumped, modrm(3, 0, reg_field(code->rhs)));
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                if (is_addrof(code->lhs) && is_reg64(code->rhs)) {
                    append_byte(dumped, rex_prefix_reg_ext(1, code->rhs,
                                                           code->lhs->lhs));
                    append_byte(dumped, 0x8b);
                    append_modrm(dumped, 2, reg_field(code->rhs),
                                 reg_field(code->lhs->lhs));
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                if (is_addrof(code->lhs) && is_reg32(code->rhs)) {
                    append_byte(dumped, rex_prefix_reg_ext(0, code->rhs,
                                                           code->lhs->lhs));
                    append_byte(dumped, 0x8b);
                    append_modrm(dumped, 2, reg_field(code->rhs),
                                 reg_field(code->lhs->lhs));
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                if (is_reg8(code->lhs) && is_addrof(code->rhs)) {
                    append_byte(dumped, rex_prefix_reg_ext(0, code->lhs,
                                                           code->rhs->lhs));
                    append_byte(dumped, 0x88);
                    append_modrm(dumped, 2, reg_field(code->lhs),
                                 reg_field(code->rhs->lhs));
                    append_dword_int(dumped, code->rhs->ival);
                    break;
                }

                if (is_reg32(code->lhs) && is_addrof(code->rhs)) {
                    append_byte(dumped, rex_prefix_reg_ext(0, code->lhs,
                                                           code->rhs->lhs));
                    append_byte(dumped, 0x89);
                    append_modrm(dumped, 2, reg_field(code->lhs),
                                 reg_field(code->rhs->lhs));
                    append_dword_int(dumped, code->rhs->ival);
                    break;
                }

                if (is_reg64(code->lhs) && is_addrof(code->rhs)) {
                    append_byte(dumped, rex_prefix_reg_ext(1, code->lhs,
                                                           code->rhs->lhs));
                    append_byte(dumped, 0x89);
                    append_modrm(dumped, 2, reg_field(code->lhs),
                                 reg_field(code->rhs->lhs));
                    append_dword_int(dumped, code->rhs->ival);
                    break;
                }

                goto not_implemented_error;

            case INST_MOVSBL:
                if (is_reg8(code->lhs) && is_reg32(code->rhs)) {
                    append_byte(dumped,
                                rex_prefix_reg_ext(0, code->rhs, code->lhs));
                    append_word(dumped, 0x0f, 0xbe);
                    append_byte(dumped, modrm(3, reg_field(code->rhs),
                                              reg_field(code->lhs)));
                    break;
                }

                if (is_addrof(code->lhs) && is_reg32(code->rhs)) {
                    append_byte(dumped, rex_prefix_reg_ext(0, code->rhs,
                                                           code->lhs->lhs));
                    append_word(dumped, 0x0f, 0xbe);
                    append_modrm(dumped, 2, reg_field(code->rhs),
                                 reg_field(code->lhs->lhs));
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                goto not_implemented_error;

            case INST_MOVSLQ:
                if (is_reg32(code->lhs) && is_reg64(code->rhs)) {
                    append_byte(dumped,
                                rex_prefix_reg_ext(1, code->rhs, code->lhs));
                    append_byte(dumped, 0x63);
                    append_byte(dumped, modrm(3, reg_field(code->rhs),
                                              reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_MOVZB:
                if (is_reg8(code->lhs) && is_reg32(code->rhs)) {
                    append_rex_prefix(dumped, 0, code->rhs, code->lhs);
                    append_word(dumped, 0x0f, 0xb6);
                    append_byte(dumped, modrm(3, reg_field(code->rhs),
                                              reg_field(code->lhs)));
                    break;
                }

                if (is_reg8(code->lhs) && is_reg64(code->rhs)) {
                    append_rex_prefix(dumped, 1, code->rhs, code->lhs);
                    append_word(dumped, 0x0f, 0xb6);
                    append_byte(dumped, modrm(3, reg_field(code->rhs),
                                              reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_ADD:
                if (is_imm(code->lhs) && is_reg64(code->rhs)) {
                    append_byte(dumped, rex_prefix_reg_ext(1, NULL, code->rhs));
                    append_byte(dumped, 0x81);
                    append_byte(dumped, modrm(3, 0, reg_field(code->rhs)));
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                if (is_reg64(code->lhs) && is_reg64(code->rhs)) {
                    append_byte(dumped,
                                rex_prefix_reg_ext(1, code->lhs, code->rhs));
                    append_byte(dumped, 0x01);
                    append_byte(dumped, modrm(3, reg_field(code->lhs),
                                              reg_field(code->rhs)));
                    break;
                }

                if (is_imm(code->lhs) && is_reg32(code->rhs)) {
                    append_rex_prefix(dumped, 0, NULL, code->rhs);
                    append_byte(dumped, 0x81);
                    append_byte(dumped, modrm(3, 0, reg_field(code->rhs)));
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    append_rex_prefix(dumped, 0, code->lhs, code->rhs);
                    append_byte(dumped, 0x01);
                    append_byte(dumped, modrm(3, reg_field(code->lhs),
                                              reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_ADDQ:
                if (is_imm(code->lhs) && is_addrof(code->rhs)) {
                    append_rex_prefix(dumped, 1, NULL, code->rhs->lhs);
                    append_byte(dumped, 0x81);
                    append_modrm(dumped, 2, 0, reg_field(code->rhs->lhs));
                    append_dword_int(dumped, code->rhs->ival);
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                goto not_implemented_error;

            case INST_SUB:
                if (is_imm(code->lhs) && is_reg64(code->rhs)) {
                    append_byte(dumped, rex_prefix_reg_ext(1, NULL, code->rhs));
                    append_byte(dumped, 0x81);
                    append_byte(dumped, modrm(3, 5, reg_field(code->rhs)));
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                if (is_reg64(code->lhs) && is_reg64(code->rhs)) {
                    append_byte(dumped,
                                rex_prefix_reg_ext(1, code->lhs, code->rhs));
                    append_byte(dumped, 0x29);
                    append_byte(dumped, modrm(3, reg_field(code->lhs),
                                              reg_field(code->rhs)));
                    break;
                }

                if (is_imm(code->lhs) && is_reg32(code->rhs)) {
                    append_rex_prefix(dumped, 0, NULL, code->rhs);
                    append_byte(dumped, 0x81);
                    append_byte(dumped, modrm(3, 5, reg_field(code->rhs)));
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    append_rex_prefix(dumped, 1, code->lhs, code->rhs);
                    append_byte(dumped, 0x29);
                    append_byte(dumped, modrm(3, reg_field(code->lhs),
                                              reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_IMUL:
                if (is_reg64(code->lhs) && is_reg64(code->rhs)) {
                    append_byte(dumped,
                                rex_prefix_reg_ext(1, code->lhs, code->rhs));
                    append_word(dumped, 0x0f, 0xaf);
                    append_byte(dumped, modrm(3, reg_field(code->rhs),
                                              reg_field(code->lhs)));
                    break;
                }

                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    append_rex_prefix(dumped, 0, code->lhs, code->rhs);
                    append_word(dumped, 0x0f, 0xaf);
                    append_byte(dumped, modrm(3, reg_field(code->rhs),
                                              reg_field(code->lhs)));
                    break;
                }

                if (is_imm(code->lhs) && is_reg64(code->rhs)) {
                    append_byte(dumped,
                                rex_prefix_reg_ext(1, code->rhs, code->rhs));
                    append_byte(dumped, 0x69);
                    append_byte(dumped, modrm(3, reg_field(code->rhs),
                                              reg_field(code->rhs)));
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                goto not_implemented_error;

            case INST_IDIV:
                if (is_reg32(code->lhs)) {
                    append_rex_prefix(dumped, 0, NULL, code->lhs);
                    append_byte(dumped, 0xf7);
                    append_byte(dumped, modrm(3, 7, reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_NEG:
                if (is_reg32(code->lhs)) {
                    append_rex_prefix(dumped, 0, NULL, code->lhs);
                    append_byte(dumped, 0xf7);
                    append_byte(dumped, modrm(3, 3, reg_field(code->lhs)));
                    break;
                }

                if (is_reg64(code->lhs)) {
                    append_rex_prefix(dumped, 1, NULL, code->lhs);
                    append_byte(dumped, 0xf7);
                    append_byte(dumped, modrm(3, 3, reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_NOT:
                if (is_reg32(code->lhs)) {
                    append_rex_prefix(dumped, 0, NULL, code->lhs);
                    append_byte(dumped, 0xf7);
                    append_byte(dumped, modrm(3, 2, reg_field(code->lhs)));
                    break;
                }

                if (is_reg64(code->lhs)) {
                    append_rex_prefix(dumped, 1, NULL, code->lhs);
                    append_byte(dumped, 0xf7);
                    append_byte(dumped, modrm(3, 2, reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_SAL:
                if (is_reg32(code->rhs)) {
                    // TODO: assume that code->lhs is CL.
                    append_rex_prefix(dumped, 0, NULL, code->rhs);
                    append_byte(dumped, 0xd3);
                    append_byte(dumped, modrm(3, 4, reg_field(code->rhs)));
                    break;
                }

                if (is_reg64(code->rhs)) {
                    // TODO: assume that code->lhs is CL.
                    append_rex_prefix(dumped, 1, NULL, code->rhs);
                    append_byte(dumped, 0xd3);
                    append_byte(dumped, modrm(3, 4, reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_SAR:
                if (is_reg32(code->rhs)) {
                    // TODO: assume that code->lhs is CL.
                    append_rex_prefix(dumped, 0, NULL, code->rhs);
                    append_byte(dumped, 0xd3);
                    append_byte(dumped, modrm(3, 7, reg_field(code->rhs)));
                    break;
                }

                if (is_reg64(code->rhs)) {
                    // TODO: assume that code->lhs is CL.
                    append_rex_prefix(dumped, 1, NULL, code->rhs);
                    append_byte(dumped, 0xd3);
                    append_byte(dumped, modrm(3, 7, reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_CMP:
                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    append_rex_prefix(dumped, 0, code->lhs, code->rhs);
                    append_byte(dumped, 0x39);
                    append_byte(dumped, modrm(3, reg_field(code->lhs),
                                              reg_field(code->rhs)));
                    break;
                }

                if (is_reg64(code->lhs) && is_reg64(code->rhs)) {
                    append_rex_prefix(dumped, 1, code->lhs, code->rhs);
                    append_byte(dumped, 0x39);
                    append_byte(dumped, modrm(3, reg_field(code->lhs),
                                              reg_field(code->rhs)));
                    break;
                }

                if (is_imm(code->lhs) && is_reg32(code->rhs)) {
                    append_rex_prefix(dumped, 0, NULL, code->rhs);
                    append_byte(dumped, 0x81);
                    append_byte(dumped, modrm(3, 7, reg_field(code->rhs)));
                    append_dword_int(dumped, code->lhs->ival);
                    break;
                }

                goto not_implemented_error;

            case INST_SETL:
                if (is_reg8(code->lhs)) {
                    append_word(dumped, 0x0f, 0x9c);
                    append_byte(dumped, modrm(3, 0, reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_SETLE:
                if (is_reg8(code->lhs)) {
                    append_word(dumped, 0x0f, 0x9e);
                    append_byte(dumped, modrm(3, 0, reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_SETE:
                if (is_reg8(code->lhs)) {
                    append_word(dumped, 0x0f, 0x94);
                    append_byte(dumped, modrm(3, 0, reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_AND:
                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    append_rex_prefix(dumped, 0, code->lhs, code->rhs);
                    append_byte(dumped, 0x21);
                    append_byte(dumped, modrm(3, reg_field(code->lhs),
                                              reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_XOR:
                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    append_rex_prefix(dumped, 0, code->lhs, code->rhs);
                    append_byte(dumped, 0x31);
                    append_byte(dumped, modrm(3, reg_field(code->lhs),
                                              reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_OR:
                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    append_rex_prefix(dumped, 0, code->lhs, code->rhs);
                    append_byte(dumped, 0x09);
                    append_byte(dumped, modrm(3, reg_field(code->lhs),
                                              reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_LEA:
                if (is_addrof(code->lhs) && is_reg64(code->rhs)) {
                    append_byte(dumped, rex_prefix_reg_ext(1, code->rhs,
                                                           code->lhs->lhs));
                    append_byte(dumped, 0x8d);
                    append_addrof(dumped, reg_field(code->rhs), code->lhs);
                    break;
                }

                goto not_implemented_error;

            case INST_PUSH:
                if (is_reg64(code->lhs)) {
                    if (is_reg_ext(code->lhs)) append_byte(dumped, 0x41);
                    append_byte(dumped, 0x50 + reg_field(code->lhs));
                    break;
                }

                goto not_implemented_error;

            case INST_POP:
                if (is_reg64(code->lhs)) {
                    if (is_reg_ext(code->lhs)) append_byte(dumped, 0x41);
                    append_byte(dumped, 0x58 + reg_field(code->lhs));
                    break;
                }

                goto not_implemented_error;

            case INST_RET:
                append_byte(dumped, 0xc3);
                break;

            case INST_CLTD:
                append_byte(dumped, 0x99);
                break;

            case INST_CLTQ:
                append_word(dumped, 0x48, 0x98);
                break;

            case INST_JMP: {
                append_byte(dumped, 0xe9);
                append_dword_int(dumped, 0);  // placeholder

                LabelPlaceholder *lph = safe_malloc(sizeof(LabelPlaceholder));
                lph->label = code->label;
                lph->offset = vector_size(dumped);
                lph->size = 4;
                vector_push_back(label_placeholders, lph);
            } break;

            case INST_JE: {
                append_byte(dumped, 0x74);
                append_byte(dumped, 0);  // placeholder

                LabelPlaceholder *lph = safe_malloc(sizeof(LabelPlaceholder));
                lph->label = code->label;
                lph->offset = vector_size(dumped);
                lph->size = 1;
                vector_push_back(label_placeholders, lph);
            } break;

            case INST_JNE: {
                append_byte(dumped, 0x75);
                append_byte(dumped, 0);  // placeholder

                LabelPlaceholder *lph = safe_malloc(sizeof(LabelPlaceholder));
                lph->label = code->label;
                lph->offset = vector_size(dumped);
                lph->size = 1;
                vector_push_back(label_placeholders, lph);
            } break;

            case INST_INCL:
                if (is_addrof(code->lhs)) {
                    append_rex_prefix(dumped, 0, NULL, code->lhs->lhs);
                    append_byte(dumped, 0xff);
                    append_addrof(dumped, 0, code->lhs);
                    break;
                }
                goto not_implemented_error;

            case INST_INCQ:
                if (is_addrof(code->lhs)) {
                    append_rex_prefix(dumped, 1, NULL, code->lhs->lhs);
                    append_byte(dumped, 0xff);
                    append_addrof(dumped, 0, code->lhs);
                    break;
                }
                goto not_implemented_error;

            case INST_DECL:
                if (is_addrof(code->lhs)) {
                    append_rex_prefix(dumped, 0, NULL, code->lhs->lhs);
                    append_byte(dumped, 0xff);
                    append_addrof(dumped, 1, code->lhs);
                    break;
                }
                goto not_implemented_error;

            case INST_DECQ:
                if (is_addrof(code->lhs)) {
                    append_rex_prefix(dumped, 1, NULL, code->lhs->lhs);
                    append_byte(dumped, 0xff);
                    append_addrof(dumped, 1, code->lhs);
                    break;
                }
                goto not_implemented_error;

            case INST_LABEL:
                map_insert(label2offset, code->label,
                           (void *)vector_size(dumped));
                break;

            case INST_OTHER:
                break;

            default:
                goto not_implemented_error;
        }

        continue;

    not_implemented_error:
        error("not implemented code: %d", code->kind);
    }

    // write offset to label placeholders
    for (int i = 0; i < vector_size(label_placeholders); i++) {
        LabelPlaceholder *lph =
            (LabelPlaceholder *)vector_get(label_placeholders, i);
        KeyValue *kv = map_lookup(label2offset, lph->label);
        assert(kv != NULL);
        int v = (int)kv_value(kv) - lph->offset;

        switch (lph->size) {
            case 4:
                vector_set(dumped, lph->offset - 4, (void *)(v & 0xff));
                vector_set(dumped, lph->offset - 3, (void *)((v >> 8) & 0xff));
                vector_set(dumped, lph->offset - 2, (void *)((v >> 16) & 0xff));
                vector_set(dumped, lph->offset - 1, (void *)((v >> 24) & 0xff));
                break;

            case 1:
                assert(-128 <= v && v <= 127);
                vector_set(dumped, lph->offset - 1, (void *)(v & 0xff));
                break;

            default:
                assert(0);
        }
    }

    ObjectImage *objimg = (ObjectImage *)safe_malloc(sizeof(ObjectImage));
    objimg->text = dumped;
    return objimg;
}

ObjectImage *assemble_code(Vector *code) { return assemble_code_detail(code); }

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
    // .text SHALL HAVE some 0x00 at the end, so roundup(..., 8) can't be used.
    int text_size = (vector_size(objimg->text) / 8 + 1) * 8;
    int sht_addr = 0x40 + text_size + 0x78 + 0x38;
    write_dword_int(fh, sht_addr);
    write_dword_int(fh, 0);
    // write_qword(fh, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

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
    write_dword_int(fh, text_size);
    write_dword_int(fh, 0);
    // write_qword(fh, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);  // size
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    int offset = 0x40 + text_size;

    // .data
    write_qword(fh, 0x21, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00);
    write_qword(fh, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_dword_int(fh, offset);
    write_dword_int(fh, 0);
    // write_qword(fh, 0x5e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .bss
    write_qword(fh, 0x27, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00);
    write_qword(fh, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_dword_int(fh, offset);
    write_dword_int(fh, 0);
    // write_qword(fh, 0x5e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .symtab
    write_qword(fh, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_dword_int(fh, offset);
    write_dword_int(fh, 0);
    // write_qword(fh, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00);
    write_qword(fh, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    offset += 0x78;

    // .strtab
    write_qword(fh, 0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_dword_int(fh, offset);
    write_dword_int(fh, 0);
    // write_qword(fh, 0xd8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    offset += 0x06;

    // .strtab
    write_qword(fh, 0x11, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_dword_int(fh, offset);
    write_dword_int(fh, 0);
    // write_qword(fh, 0xde, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    write_qword(fh, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}
