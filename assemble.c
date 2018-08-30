#include "aqcc.h"

void add_byte(Vector *vec, int val) { vector_push_back(vec, (void *)val); }

void set_byte(Vector *vec, int index, int val)
{
    vector_set(vec, index, (void *)val);
}

void add_word(Vector *vec, int val0, int val1)
{
    add_byte(vec, val0);
    add_byte(vec, val1);
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

ObjectImage *target_objimg = NULL;

void init_target_objimg(ObjectImage *objimg) { target_objimg = objimg; }

int get_target_text_size() { return vector_size(target_objimg->text); }

typedef struct {
    int offset, symtabidx, type;
    char *label;
} RelaEntry;

void add_rela_entry(int offset, int symtabidx, int type, char *label)
{
    RelaEntry *entry = (RelaEntry *)safe_malloc(sizeof(RelaEntry));
    entry->offset = offset;
    entry->symtabidx = symtabidx;
    entry->type = type;
    entry->label = label;

    vector_push_back(target_objimg->rela, entry);
}

void retext_byte(int index, int val0)
{
    vector_set(target_objimg->text, index, (void *)val0);
}

void text_byte(int val0) { add_byte(target_objimg->text, val0); }

void text_word(int val0, int val1)
{
    text_byte(val0);
    text_byte(val1);
}

void text_dword(int val0, int val1, int val2, int val3)
{
    text_word(val0, val1);
    text_word(val2, val3);
}

void text_dword_int(int ival)
{
    text_dword(ival & 0xff, (ival >> 8) & 0xff, (ival >> 16) & 0xff,
               (ival >> 24) & 0xff);
}

void text_qword(int val0, int val1, int val2, int val3, int val4, int val5,
                int val6, int val7)
{
    text_dword(val0, val1, val2, val3);
    text_dword(val4, val5, val6, val7);
}

void text_string(char *src, int len)
{
    add_string(target_objimg->text, src, len);
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

int is_addrof(Code *code)
{
    return code->kind == CD_ADDR_OF || code->kind == CD_ADDR_OF_LABEL;
}

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

int text_modrm(int mod, int reg, int rm)
{
    text_byte(modrm(mod, reg, rm));

    // If mod == 2 and rm == 4 then SIB byte is enabled.
    // If I == 0 in SIB then index is disabled.
    // That's why this line is needed.
    // NOT CARGO CULT PROGRAMMING!!
    // But this spec is awful, isn't it :thinking_face:
    if (mod == 2 && rm == 4) text_byte(modrm(0, 4, 4));
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

void text_rex_prefix(int is64, Code *reg, Code *rm)
{
    int pre = rex_prefix_reg_ext(is64, reg, rm);
    if (pre == 0x40) return;  // no info
    text_byte(pre);
}

void text_addrof(int reg, Code *mem)
{
    assert(is_addrof(mem));
    text_modrm(2, reg, reg_field(mem->lhs));

    switch (mem->kind) {
        case CD_ADDR_OF:
            text_dword_int(mem->ival);
            break;

        case CD_ADDR_OF_LABEL:
            add_rela_entry(get_target_text_size(), 2, 2, mem->label);
            text_dword_int(0);
            break;
    }
}

ObjectImage *assemble_code_detail(Vector *code_list)
{
    // all data are stored in this variable.
    ObjectImage *objimg = (ObjectImage *)safe_malloc(sizeof(ObjectImage));
    objimg->text = new_vector();
    objimg->rela = new_vector();
    init_target_objimg(objimg);

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
                    text_byte(rex_prefix_reg_ext(1, code->lhs, code->rhs));
                    text_byte(0x89);
                    text_byte(
                        modrm(3, reg_field(code->lhs), reg_field(code->rhs)));
                    break;
                }

                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    text_rex_prefix(0, code->lhs, code->rhs);
                    text_byte(0x89);
                    text_byte(
                        modrm(3, reg_field(code->lhs), reg_field(code->rhs)));
                    break;
                }

                if (is_reg8(code->lhs) && is_reg8(code->rhs)) {
                    text_rex_prefix(0, code->lhs, code->rhs);
                    text_byte(0x88);
                    text_byte(
                        modrm(3, reg_field(code->lhs), reg_field(code->rhs)));
                    break;
                }

                if (is_imm(code->lhs) && is_reg32(code->rhs)) {
                    if (is_reg_ext(code->rhs)) text_byte(0x41);
                    text_byte(0xb8 + reg_field(code->rhs));
                    text_dword_int(code->lhs->ival);
                    break;
                }

                if (is_imm(code->lhs) && is_reg64(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(1, NULL, code->rhs));
                    text_byte(0xc7);
                    text_byte(modrm(3, 0, reg_field(code->rhs)));
                    text_dword_int(code->lhs->ival);
                    break;
                }

                if (is_addrof(code->lhs) && is_reg64(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(1, code->rhs, code->lhs->lhs));
                    text_byte(0x8b);
                    text_modrm(2, reg_field(code->rhs),
                               reg_field(code->lhs->lhs));
                    text_dword_int(code->lhs->ival);
                    break;
                }

                if (is_addrof(code->lhs) && is_reg32(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(0, code->rhs, code->lhs->lhs));
                    text_byte(0x8b);
                    text_modrm(2, reg_field(code->rhs),
                               reg_field(code->lhs->lhs));
                    text_dword_int(code->lhs->ival);
                    break;
                }

                if (is_reg8(code->lhs) && is_addrof(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(0, code->lhs, code->rhs->lhs));
                    text_byte(0x88);
                    text_modrm(2, reg_field(code->lhs),
                               reg_field(code->rhs->lhs));
                    text_dword_int(code->rhs->ival);
                    break;
                }

                if (is_reg32(code->lhs) && is_addrof(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(0, code->lhs, code->rhs->lhs));
                    text_byte(0x89);
                    text_modrm(2, reg_field(code->lhs),
                               reg_field(code->rhs->lhs));
                    text_dword_int(code->rhs->ival);
                    break;
                }

                if (is_reg64(code->lhs) && is_addrof(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(1, code->lhs, code->rhs->lhs));
                    text_byte(0x89);
                    text_modrm(2, reg_field(code->lhs),
                               reg_field(code->rhs->lhs));
                    text_dword_int(code->rhs->ival);
                    break;
                }

                goto not_implemented_error;

            case INST_MOVSBL:
                if (is_reg8(code->lhs) && is_reg32(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(0, code->rhs, code->lhs));
                    text_word(0x0f, 0xbe);
                    text_byte(
                        modrm(3, reg_field(code->rhs), reg_field(code->lhs)));
                    break;
                }

                if (is_addrof(code->lhs) && is_reg32(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(0, code->rhs, code->lhs->lhs));
                    text_word(0x0f, 0xbe);
                    text_modrm(2, reg_field(code->rhs),
                               reg_field(code->lhs->lhs));
                    text_dword_int(code->lhs->ival);
                    break;
                }

                goto not_implemented_error;

            case INST_MOVSLQ:
                if (is_reg32(code->lhs) && is_reg64(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(1, code->rhs, code->lhs));
                    text_byte(0x63);
                    text_byte(
                        modrm(3, reg_field(code->rhs), reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_MOVZB:
                if (is_reg8(code->lhs) && is_reg32(code->rhs)) {
                    text_rex_prefix(0, code->rhs, code->lhs);
                    text_word(0x0f, 0xb6);
                    text_byte(
                        modrm(3, reg_field(code->rhs), reg_field(code->lhs)));
                    break;
                }

                if (is_reg8(code->lhs) && is_reg64(code->rhs)) {
                    text_rex_prefix(1, code->rhs, code->lhs);
                    text_word(0x0f, 0xb6);
                    text_byte(
                        modrm(3, reg_field(code->rhs), reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_ADD:
                if (is_imm(code->lhs) && is_reg64(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(1, NULL, code->rhs));
                    text_byte(0x81);
                    text_byte(modrm(3, 0, reg_field(code->rhs)));
                    text_dword_int(code->lhs->ival);
                    break;
                }

                if (is_reg64(code->lhs) && is_reg64(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(1, code->lhs, code->rhs));
                    text_byte(0x01);
                    text_byte(
                        modrm(3, reg_field(code->lhs), reg_field(code->rhs)));
                    break;
                }

                if (is_imm(code->lhs) && is_reg32(code->rhs)) {
                    text_rex_prefix(0, NULL, code->rhs);
                    text_byte(0x81);
                    text_byte(modrm(3, 0, reg_field(code->rhs)));
                    text_dword_int(code->lhs->ival);
                    break;
                }

                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    text_rex_prefix(0, code->lhs, code->rhs);
                    text_byte(0x01);
                    text_byte(
                        modrm(3, reg_field(code->lhs), reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_ADDQ:
                if (is_imm(code->lhs) && is_addrof(code->rhs)) {
                    text_rex_prefix(1, NULL, code->rhs->lhs);
                    text_byte(0x81);
                    text_modrm(2, 0, reg_field(code->rhs->lhs));
                    text_dword_int(code->rhs->ival);
                    text_dword_int(code->lhs->ival);
                    break;
                }

                goto not_implemented_error;

            case INST_SUB:
                if (is_imm(code->lhs) && is_reg64(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(1, NULL, code->rhs));
                    text_byte(0x81);
                    text_byte(modrm(3, 5, reg_field(code->rhs)));
                    text_dword_int(code->lhs->ival);
                    break;
                }

                if (is_reg64(code->lhs) && is_reg64(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(1, code->lhs, code->rhs));
                    text_byte(0x29);
                    text_byte(
                        modrm(3, reg_field(code->lhs), reg_field(code->rhs)));
                    break;
                }

                if (is_imm(code->lhs) && is_reg32(code->rhs)) {
                    text_rex_prefix(0, NULL, code->rhs);
                    text_byte(0x81);
                    text_byte(modrm(3, 5, reg_field(code->rhs)));
                    text_dword_int(code->lhs->ival);
                    break;
                }

                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    text_rex_prefix(1, code->lhs, code->rhs);
                    text_byte(0x29);
                    text_byte(
                        modrm(3, reg_field(code->lhs), reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_IMUL:
                if (is_reg64(code->lhs) && is_reg64(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(1, code->lhs, code->rhs));
                    text_word(0x0f, 0xaf);
                    text_byte(
                        modrm(3, reg_field(code->rhs), reg_field(code->lhs)));
                    break;
                }

                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    text_rex_prefix(0, code->lhs, code->rhs);
                    text_word(0x0f, 0xaf);
                    text_byte(
                        modrm(3, reg_field(code->rhs), reg_field(code->lhs)));
                    break;
                }

                if (is_imm(code->lhs) && is_reg64(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(1, code->rhs, code->rhs));
                    text_byte(0x69);
                    text_byte(
                        modrm(3, reg_field(code->rhs), reg_field(code->rhs)));
                    text_dword_int(code->lhs->ival);
                    break;
                }

                goto not_implemented_error;

            case INST_IDIV:
                if (is_reg32(code->lhs)) {
                    text_rex_prefix(0, NULL, code->lhs);
                    text_byte(0xf7);
                    text_byte(modrm(3, 7, reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_NEG:
                if (is_reg32(code->lhs)) {
                    text_rex_prefix(0, NULL, code->lhs);
                    text_byte(0xf7);
                    text_byte(modrm(3, 3, reg_field(code->lhs)));
                    break;
                }

                if (is_reg64(code->lhs)) {
                    text_rex_prefix(1, NULL, code->lhs);
                    text_byte(0xf7);
                    text_byte(modrm(3, 3, reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_NOT:
                if (is_reg32(code->lhs)) {
                    text_rex_prefix(0, NULL, code->lhs);
                    text_byte(0xf7);
                    text_byte(modrm(3, 2, reg_field(code->lhs)));
                    break;
                }

                if (is_reg64(code->lhs)) {
                    text_rex_prefix(1, NULL, code->lhs);
                    text_byte(0xf7);
                    text_byte(modrm(3, 2, reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_SAL:
                if (is_reg32(code->rhs)) {
                    // TODO: assume that code->lhs is CL.
                    text_rex_prefix(0, NULL, code->rhs);
                    text_byte(0xd3);
                    text_byte(modrm(3, 4, reg_field(code->rhs)));
                    break;
                }

                if (is_reg64(code->rhs)) {
                    // TODO: assume that code->lhs is CL.
                    text_rex_prefix(1, NULL, code->rhs);
                    text_byte(0xd3);
                    text_byte(modrm(3, 4, reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_SAR:
                if (is_reg32(code->rhs)) {
                    // TODO: assume that code->lhs is CL.
                    text_rex_prefix(0, NULL, code->rhs);
                    text_byte(0xd3);
                    text_byte(modrm(3, 7, reg_field(code->rhs)));
                    break;
                }

                if (is_reg64(code->rhs)) {
                    // TODO: assume that code->lhs is CL.
                    text_rex_prefix(1, NULL, code->rhs);
                    text_byte(0xd3);
                    text_byte(modrm(3, 7, reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_CMP:
                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    text_rex_prefix(0, code->lhs, code->rhs);
                    text_byte(0x39);
                    text_byte(
                        modrm(3, reg_field(code->lhs), reg_field(code->rhs)));
                    break;
                }

                if (is_reg64(code->lhs) && is_reg64(code->rhs)) {
                    text_rex_prefix(1, code->lhs, code->rhs);
                    text_byte(0x39);
                    text_byte(
                        modrm(3, reg_field(code->lhs), reg_field(code->rhs)));
                    break;
                }

                if (is_imm(code->lhs) && is_reg32(code->rhs)) {
                    text_rex_prefix(0, NULL, code->rhs);
                    text_byte(0x81);
                    text_byte(modrm(3, 7, reg_field(code->rhs)));
                    text_dword_int(code->lhs->ival);
                    break;
                }

                goto not_implemented_error;

            case INST_SETL:
                if (is_reg8(code->lhs)) {
                    text_word(0x0f, 0x9c);
                    text_byte(modrm(3, 0, reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_SETLE:
                if (is_reg8(code->lhs)) {
                    text_word(0x0f, 0x9e);
                    text_byte(modrm(3, 0, reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_SETE:
                if (is_reg8(code->lhs)) {
                    text_word(0x0f, 0x94);
                    text_byte(modrm(3, 0, reg_field(code->lhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_AND:
                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    text_rex_prefix(0, code->lhs, code->rhs);
                    text_byte(0x21);
                    text_byte(
                        modrm(3, reg_field(code->lhs), reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_XOR:
                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    text_rex_prefix(0, code->lhs, code->rhs);
                    text_byte(0x31);
                    text_byte(
                        modrm(3, reg_field(code->lhs), reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_OR:
                if (is_reg32(code->lhs) && is_reg32(code->rhs)) {
                    text_rex_prefix(0, code->lhs, code->rhs);
                    text_byte(0x09);
                    text_byte(
                        modrm(3, reg_field(code->lhs), reg_field(code->rhs)));
                    break;
                }

                goto not_implemented_error;

            case INST_LEA:
                if (is_addrof(code->lhs) && is_reg64(code->rhs)) {
                    text_byte(rex_prefix_reg_ext(1, code->rhs, code->lhs->lhs));
                    text_byte(0x8d);
                    text_addrof(reg_field(code->rhs), code->lhs);
                    break;
                }

                goto not_implemented_error;

            case INST_PUSH:
                if (is_reg64(code->lhs)) {
                    if (is_reg_ext(code->lhs)) text_byte(0x41);
                    text_byte(0x50 + reg_field(code->lhs));
                    break;
                }

                goto not_implemented_error;

            case INST_POP:
                if (is_reg64(code->lhs)) {
                    if (is_reg_ext(code->lhs)) text_byte(0x41);
                    text_byte(0x58 + reg_field(code->lhs));
                    break;
                }

                goto not_implemented_error;

            case INST_RET:
                text_byte(0xc3);
                break;

            case INST_CLTD:
                text_byte(0x99);
                break;

            case INST_CLTQ:
                text_word(0x48, 0x98);
                break;

            case INST_JMP: {
                text_byte(0xe9);
                text_dword_int(0);  // placeholder

                LabelPlaceholder *lph = safe_malloc(sizeof(LabelPlaceholder));
                lph->label = code->label;
                lph->offset = get_target_text_size();
                lph->size = 4;
                vector_push_back(label_placeholders, lph);
            } break;

            case INST_JE: {
                text_byte(0x74);
                text_byte(0);  // placeholder

                LabelPlaceholder *lph = safe_malloc(sizeof(LabelPlaceholder));
                lph->label = code->label;
                lph->offset = get_target_text_size();
                lph->size = 1;
                vector_push_back(label_placeholders, lph);
            } break;

            case INST_JNE: {
                text_byte(0x75);
                text_byte(0);  // placeholder

                LabelPlaceholder *lph = safe_malloc(sizeof(LabelPlaceholder));
                lph->label = code->label;
                lph->offset = get_target_text_size();
                lph->size = 1;
                vector_push_back(label_placeholders, lph);
            } break;

            case INST_INCL:
                if (is_addrof(code->lhs)) {
                    text_rex_prefix(0, NULL, code->lhs->lhs);
                    text_byte(0xff);
                    text_addrof(0, code->lhs);
                    break;
                }
                goto not_implemented_error;

            case INST_INCQ:
                if (is_addrof(code->lhs)) {
                    text_rex_prefix(1, NULL, code->lhs->lhs);
                    text_byte(0xff);
                    text_addrof(0, code->lhs);
                    break;
                }
                goto not_implemented_error;

            case INST_DECL:
                if (is_addrof(code->lhs)) {
                    text_rex_prefix(0, NULL, code->lhs->lhs);
                    text_byte(0xff);
                    text_addrof(1, code->lhs);
                    break;
                }
                goto not_implemented_error;

            case INST_DECQ:
                if (is_addrof(code->lhs)) {
                    text_rex_prefix(1, NULL, code->lhs->lhs);
                    text_byte(0xff);
                    text_addrof(1, code->lhs);
                    break;
                }
                goto not_implemented_error;

            case INST_LABEL:
                map_insert(label2offset, code->label,
                           (void *)get_target_text_size());
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
                retext_byte(lph->offset - 4, v & 0xff);
                retext_byte(lph->offset - 3, (v >> 8) & 0xff);
                retext_byte(lph->offset - 2, (v >> 16) & 0xff);
                retext_byte(lph->offset - 1, (v >> 24) & 0xff);
                break;

            case 1:
                assert(-128 <= v && v <= 127);
                retext_byte(lph->offset - 1, v & 0xff);
                break;

            default:
                assert(0);
        }
    }

    return objimg;
}

ObjectImage *assemble_code(Vector *code) { return assemble_code_detail(code); }

void dump_object_image(ObjectImage *objimg, FILE *fh)
{
    Vector *dumped = new_vector();

    //
    // *** ELF HEADRE ***
    //

    int header_offset = 0;

    // ELF magic number
    add_dword(dumped, 0x7f, 0x45, 0x4c, 0x46);
    // 64bit
    add_byte(dumped, 0x02);
    // little endian
    add_byte(dumped, 0x01);
    // original version of ELF
    add_byte(dumped, 0x01);
    // System V
    add_byte(dumped, 0x00);
    // padding
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    // ET_REL
    add_word(dumped, 0x01, 0x00);
    // x86-64
    add_word(dumped, 0x3e, 0x00);
    // original version of ELF
    add_dword(dumped, 0x01, 0x00, 0x00, 0x00);

    // addr of entry point
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    // addr of program header table
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    // addr of section header table (placeholder)
    int sht_addr = vector_size(dumped);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // flag
    add_dword(dumped, 0x00, 0x00, 0x00, 0x00);

    // size of this header
    add_word(dumped, 0x40, 0x00);

    // size of program header table entry
    add_word(dumped, 0x00, 0x00);
    // number of entries in program header table
    add_word(dumped, 0x00, 0x00);

    // size of section header table entry
    add_word(dumped, 0x40, 0x00);
    // number of entries in section header table
    add_word(dumped, 0x07, 0x00);
    // index of section header entry containing section names
    add_word(dumped, 0x06, 0x00);

    int header_size = vector_size(dumped) - header_offset;

    //
    // *** PROGRAM ***
    //

    int text_offset = vector_size(dumped);

    // .text
    for (int i = 0; i < vector_size(objimg->text); i++)
        add_byte(dumped, (int)vector_get(objimg->text, i));
    // padding
    // NOTE: .text SHALL HAVE some 0x00 at the end.
    for (int i = 0; i < 8 - vector_size(objimg->text) % 8; i++)
        add_byte(dumped, 0x00);
    int text_size = vector_size(dumped) - text_offset;
    assert(text_size % 8 == 0);

    // .data
    int data_offset = vector_size(dumped);
    int data_size = vector_size(dumped) - data_offset;

    // .bss
    int bss_offset = vector_size(dumped);
    int bss_size = vector_size(dumped) - bss_offset;

    // .symtab
    int symtab_offset = vector_size(dumped);

    add_dword(dumped, 0x00, 0x00, 0x00, 0x00);  // st_name
    add_byte(dumped, 0x00);                     // st_info
    add_byte(dumped, 0x00);                     // st_other
    add_word(dumped, 0x00, 0x00);               // st_shndx
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00);  // st_value
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00);  // st_size

    add_dword(dumped, 0x00, 0x00, 0x00, 0x00);  // st_name
    add_byte(dumped, 0x03);                     // st_info
    add_byte(dumped, 0x00);                     // st_other
    add_word(dumped, 0x01, 0x00);               // st_shndx
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00);  // st_value
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00);  // st_size

    add_dword(dumped, 0x00, 0x00, 0x00, 0x00);  // st_name
    add_byte(dumped, 0x03);                     // st_info
    add_byte(dumped, 0x00);                     // st_other
    add_word(dumped, 0x02, 0x00);               // st_shndx
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00);  // st_value
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00);  // st_size

    add_dword(dumped, 0x00, 0x00, 0x00, 0x00);  // st_name
    add_byte(dumped, 0x03);                     // st_info
    add_byte(dumped, 0x00);                     // st_other
    add_word(dumped, 0x03, 0x00);               // st_shndx
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00);  // st_value
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00);  // st_size

    add_dword(dumped, 0x01, 0x00, 0x00, 0x00);  // st_name
    add_byte(dumped, 0x10);                     // st_info
    add_byte(dumped, 0x00);                     // st_other
    add_word(dumped, 0x01, 0x00);               // st_shndx
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00);  // st_value
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00);  // st_size

    int symtab_size = vector_size(dumped) - symtab_offset;

    // .strtab
    int strtab0_offset = vector_size(dumped);

    add_string(dumped, "\0main\0", 6);

    int strtab0_size = vector_size(dumped) - strtab0_offset;

    // .strtab
    int strtab1_offset = vector_size(dumped);

    add_byte(dumped, 0x00);
    add_string(dumped, ".symtab\0", 8);
    add_string(dumped, ".strtab\0", 8);
    add_string(dumped, ".shstrtab\0", 10);
    add_string(dumped, ".text\0", 6);
    add_string(dumped, ".data\0", 6);
    add_string(dumped, ".bss\0", 5);
    add_string(dumped, "\0\0\0\0\0\0", 6);

    int strtab1_size = vector_size(dumped) - strtab1_offset;

    //
    // *** SECTION HEADER ***
    //

    int sht_offset = vector_size(dumped);

    // write sht_offset to header
    set_byte(dumped, sht_addr + 0, (sht_offset >> 0) & 0xff);
    set_byte(dumped, sht_addr + 1, (sht_offset >> 8) & 0xff);
    set_byte(dumped, sht_addr + 2, (sht_offset >> 16) & 0xff);
    set_byte(dumped, sht_addr + 3, (sht_offset >> 24) & 0xff);
    set_byte(dumped, sht_addr + 4, 0);
    set_byte(dumped, sht_addr + 5, 0);
    set_byte(dumped, sht_addr + 6, 0);
    set_byte(dumped, sht_addr + 7, 0);

    // NULL
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .text
    add_qword(dumped, 0x1b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword_int(dumped, text_offset, 0);
    add_qword_int(dumped, text_size, 0);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .data
    add_qword(dumped, 0x21, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword_int(dumped, text_offset, 0);
    add_qword_int(dumped, text_size, 0);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .bss
    add_qword(dumped, 0x27, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword_int(dumped, bss_offset, 0);
    add_qword_int(dumped, bss_size, 0);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .symtab
    add_qword(dumped, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword_int(dumped, symtab_offset, 0);
    add_qword_int(dumped, symtab_size, 0);
    add_qword(dumped, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .strtab
    add_qword(dumped, 0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword_int(dumped, strtab0_offset, 0);
    add_qword_int(dumped, strtab0_size, 0);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // .strtab
    add_qword(dumped, 0x11, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword_int(dumped, strtab1_offset, 0);
    add_qword_int(dumped, strtab1_size, 0);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    add_qword(dumped, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    int sht_size = vector_size(dumped) - sht_offset;

    // write dumped to file
    for (int i = 0; i < vector_size(dumped); i++)
        write_byte(fh, (int)vector_get(dumped, i));
}
