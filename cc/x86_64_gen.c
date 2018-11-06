#include "cc.h"

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

struct Code {
    int kind;

    Code *lhs, *rhs;
    int ival;
    char *sval;  // size is ival
    char *label;
    Vector *read_dep;
    int can_be_eliminated;
};

static Code *new_code(int kind)
{
    Code *code = safe_malloc(sizeof(Code));
    code->kind = kind;
    code->lhs = code->rhs = NULL;
    code->ival = 0;
    code->sval = NULL;
    code->label = NULL;
    code->read_dep = new_vector();
    code->can_be_eliminated = 1;
    return code;
}

static Code *new_binop_code(int kind, Code *lhs, Code *rhs)
{
    Code *code = new_code(kind);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    vector_push_back(code->read_dep, rhs);
    return code;
}

static Code *new_unary_code(int kind, Code *lhs)
{
    Code *code = new_code(kind);
    code->lhs = lhs;
    code->rhs = NULL;
    vector_push_back(code->read_dep, lhs);
    return code;
}

static Code *new_value_code(int value)
{
    Code *code = new_code(CD_VALUE);
    code->ival = value;
    return code;
}

static Code *new_addrof_label_code(Code *reg, char *label)
{
    Code *code = new_code(CD_ADDR_OF_LABEL);
    code->lhs = reg;
    code->label = label;
    return code;
}

static Code *new_addrof_code(Code *reg, int offset)
{
    Code *code = new_code(CD_ADDR_OF);
    code->lhs = reg;
    code->ival = offset;
    return code;
}

static int is_register_code(Code *code)
{
    if (code == NULL) return 0;
    return !(code->kind & INST_) &&
           code->kind & (REG_8 | REG_16 | REG_32 | REG_64);
}

static int reg_of_nbyte(int nbyte, int reg)
{
    switch (nbyte) {
        case 1:
            return (reg & 31) | REG_8;
        case 2:
            return (reg & 31) | REG_16;
        case 4:
            return (reg & 31) | REG_32;
        case 8:
            return (reg & 31) | REG_64;
    }
    assert(0);
}

static Code *nbyte_reg(int nbyte, int reg)
{
    return new_code(reg_of_nbyte(nbyte, reg));
}

static int alignment_of(Type *type)
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
        case TY_UNION:
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

static Code *MOV(Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_MOV);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    if (!is_register_code(rhs)) vector_push_back(code->read_dep, rhs);
    return code;
}

static Code *MOVL(Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_MOVL);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    if (!is_register_code(rhs)) vector_push_back(code->read_dep, rhs);
    return code;
}

static Code *MOVSBL(Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_MOVSBL);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    if (!is_register_code(rhs)) vector_push_back(code->read_dep, rhs);
    return code;
}

static Code *MOVSLQ(Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_MOVSLQ);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    if (!is_register_code(rhs)) vector_push_back(code->read_dep, rhs);
    return code;
}

static Code *MOVZB(Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_MOVZB);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    if (!is_register_code(rhs)) vector_push_back(code->read_dep, rhs);
    return code;
}

static Code *LEA(Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_LEA);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    if (!is_register_code(rhs)) vector_push_back(code->read_dep, rhs);
    return code;
}

static Code *PUSH(Code *lhs) { return new_unary_code(INST_PUSH, lhs); }

static Code *POP(Code *lhs) { return new_unary_code(INST_POP, lhs); }

static Code *ADD(Code *lhs, Code *rhs)
{
    return new_binop_code(INST_ADD, lhs, rhs);
}

static Code *ADDQ(Code *lhs, Code *rhs)
{
    return new_binop_code(INST_ADDQ, lhs, rhs);
}

static Code *SUB(Code *lhs, Code *rhs)
{
    return new_binop_code(INST_SUB, lhs, rhs);
}

static Code *IMUL(Code *lhs, Code *rhs)
{
    return new_binop_code(INST_IMUL, lhs, rhs);
}

static Code *IDIV(Code *lhs) { return new_unary_code(INST_IDIV, lhs); }

static Code *SAR(Code *lhs, Code *rhs)
{
    return new_binop_code(INST_SAR, lhs, rhs);
}

static Code *SAL(Code *lhs, Code *rhs)
{
    return new_binop_code(INST_SAL, lhs, rhs);
}

static Code *NEG(Code *lhs) { return new_unary_code(INST_NEG, lhs); }

static Code *NOT(Code *lhs) { return new_unary_code(INST_NOT, lhs); }

static Code *CMP(Code *lhs, Code *rhs)
{
    return new_binop_code(INST_CMP, lhs, rhs);
}

static Code *SETL(Code *lhs) { return new_unary_code(INST_SETL, lhs); }

static Code *SETLE(Code *lhs) { return new_unary_code(INST_SETLE, lhs); }

static Code *SETE(Code *lhs) { return new_unary_code(INST_SETE, lhs); }

static Code *AND(Code *lhs, Code *rhs)
{
    return new_binop_code(INST_AND, lhs, rhs);
}

static Code *XOR(Code *lhs, Code *rhs)
{
    return new_binop_code(INST_XOR, lhs, rhs);
}

static Code *OR(Code *lhs, Code *rhs)
{
    return new_binop_code(INST_OR, lhs, rhs);
}

static Code *RET() { return new_code(INST_RET); }

static Code *CLTD() { return new_code(INST_CLTD); }

static Code *CLTQ() { return new_code(INST_CLTQ); }

static Code *JMP(char *label)
{
    Code *code = new_code(INST_JMP);
    code->label = label;
    return code;
}

static Code *JE(char *label)
{
    Code *code = new_code(INST_JE);
    code->label = label;
    return code;
}

static Code *JNE(char *label)
{
    Code *code = new_code(INST_JNE);
    code->label = label;
    return code;
}

static Code *JAE(char *label)
{
    Code *code = new_code(INST_JAE);
    code->label = label;
    return code;
}

static Code *LABEL(char *label)
{
    Code *code = new_code(INST_LABEL);
    code->label = label;
    return code;
}

static Code *INCL(Code *lhs) { return new_unary_code(INST_INCL, lhs); }

static Code *INCQ(Code *lhs) { return new_unary_code(INST_INCQ, lhs); }

static Code *DECL(Code *lhs) { return new_unary_code(INST_DECL, lhs); }

static Code *DECQ(Code *lhs) { return new_unary_code(INST_DECQ, lhs); }

static Code *EAX() { return new_code(REG_EAX); }

static Code *EDX() { return new_code(REG_EDX); }

static Code *RAX() { return new_code(REG_RAX); }

static Code *RBP() { return new_code(REG_RBP); }

static Code *RSP() { return new_code(REG_RSP); }

static Code *RIP() { return new_code(REG_RIP); }

static Code *RDI() { return new_code(REG_RDI); }

static Code *RDX() { return new_code(REG_RDX); }

static Code *R10() { return new_code(REG_R10); }

static Code *R11() { return new_code(REG_R11); }

static Code *R12() { return new_code(REG_R12); }

static Code *R13() { return new_code(REG_R13); }

static Code *R14() { return new_code(REG_R14); }

static Code *R15() { return new_code(REG_R15); }

static Code *AL() { return new_code(REG_AL); }

static Code *CL() { return new_code(REG_CL); }

static Code *GLOBAL(char *label)
{
    Code *code = new_code(CD_GLOBAL);
    code->label = label;
    return code;
}

static char *code2str(Code *code)
{
    if (code == NULL) return NULL;

    switch (code->kind) {
        case REG_AL:
            return "%al";
        case REG_DIL:
            return "%dil";
        case REG_SIL:
            return "%sil";
        case REG_DL:
            return "%dl";
        case REG_CL:
            return "%cl";
        case REG_R8B:
            return "%r8b";
        case REG_R9B:
            return "%r9b";
        case REG_R10B:
            return "%r10b";
        case REG_R11B:
            return "%r11b";
        case REG_R12B:
            return "%r12b";
        case REG_R13B:
            return "%r13b";
        case REG_R14B:
            return "%r14b";
        case REG_R15B:
            return "%r15b";

        case REG_AX:
            return "%ax";
        case REG_DI:
            return "%di";
        case REG_SI:
            return "%si";
        case REG_DX:
            return "%dx";
        case REG_CX:
            return "%cx";
        case REG_R8W:
            return "%r8w";
        case REG_R9W:
            return "%r9w";
        case REG_R10W:
            return "%r10w";
        case REG_R11W:
            return "%r11w";
        case REG_R12W:
            return "%r12w";
        case REG_R13W:
            return "%r13w";
        case REG_R14W:
            return "%r14w";
        case REG_R15W:
            return "%r15w";

        case REG_EAX:
            return "%eax";
        case REG_EDI:
            return "%edi";
        case REG_ESI:
            return "%esi";
        case REG_EDX:
            return "%edx";
        case REG_ECX:
            return "%ecx";
        case REG_R8D:
            return "%r8d";
        case REG_R9D:
            return "%r9d";
        case REG_R10D:
            return "%r10d";
        case REG_R11D:
            return "%r11d";
        case REG_R12D:
            return "%r12d";
        case REG_R13D:
            return "%r13d";
        case REG_R14D:
            return "%r14d";
        case REG_R15D:
            return "%r15d";

        case REG_RAX:
            return "%rax";
        case REG_RDI:
            return "%rdi";
        case REG_RSI:
            return "%rsi";
        case REG_RDX:
            return "%rdx";
        case REG_RCX:
            return "%rcx";
        case REG_R8:
            return "%r8";
        case REG_R9:
            return "%r9";
        case REG_R10:
            return "%r10";
        case REG_R11:
            return "%r11";
        case REG_R12:
            return "%r12";
        case REG_R13:
            return "%r13";
        case REG_R14:
            return "%r14";
        case REG_R15:
            return "%r15";
        case REG_RBP:
            return "%rbp";
        case REG_RSP:
            return "%rsp";

        case REG_RIP:
            return "%rip";

        case INST_MOV:
            return format("mov %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_MOVL:
            return format("movl %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_MOVSBL:
            return format("movsbl %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_MOVSLQ:
            return format("movslq %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_MOVZB:
            return format("movzb %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_LEA:
            return format("lea %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_POP:
            return format("pop %s", code2str(code->lhs));

        case INST_PUSH:
            return format("push %s", code2str(code->lhs));

        case INST_ADD:
            return format("add %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_ADDQ:
            return format("addq %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_SUB:
            return format("sub %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_IMUL:
            return format("imul %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_IDIV:
            return format("idiv %s", code2str(code->lhs));

        case INST_SAR:
            return format("sar %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_SAL:
            return format("sal %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_NEG:
            return format("neg %s", code2str(code->lhs));

        case INST_NOT:
            return format("not %s", code2str(code->lhs));

        case INST_CMP:
            return format("cmp %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_SETL:
            return format("setl %s", code2str(code->lhs));

        case INST_SETLE:
            return format("setle %s", code2str(code->lhs));

        case INST_SETE:
            return format("sete %s", code2str(code->lhs));

        case INST_AND:
            return format("and %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_XOR:
            return format("xor %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_OR:
            return format("or %s, %s", code2str(code->lhs),
                          code2str(code->rhs));

        case INST_RET:
            return "ret";

        case INST_CLTD:
            return "cltd";

        case INST_CLTQ:
            return "cltq";

        case INST_JMP:
            return format("jmp %s", code->label);

        case INST_JE:
            return format("je %s", code->label);

        case INST_JNE:
            return format("jne %s", code->label);

        case INST_JAE:
            return format("jae %s", code->label);

        case INST_LABEL:
            return format("%s:", code->label);

        case INST_INCL:
            return format("incl %s", code2str(code->lhs));

        case INST_INCQ:
            return format("incq %s", code2str(code->lhs));

        case INST_DECL:
            return format("decl %s", code2str(code->lhs));

        case INST_DECQ:
            return format("decq %s", code2str(code->lhs));

        case INST_CALL:
            return format("call %s", code->label);

        case INST_NOP:
            return "nop";

        case INST_SYSCALL:
            return "syscall";

        case CD_COMMENT:
            return format("/* %s */", code->sval);

        case CD_VALUE:
            return format("$%d", code->ival);

        case CD_ADDR_OF:
            if (code->ival == 0) return format("(%s)", code2str(code->lhs));
            return format("%d(%s)", code->ival, code2str(code->lhs));

        case CD_ADDR_OF_LABEL:
            return format("%s(%s)", code->label, code2str(code->lhs));

        case CD_GLOBAL:
            return format(".global %s", code->label);

        case CD_TEXT:
            return ".text";

        case CD_DATA:
            return ".data";

        case CD_ZERO:
            return format(".zero %d", code->ival);

        case CD_LONG:
            return format(".long %d", code->ival);

        case CD_BYTE:
            return format(".byte %d", code->ival);

        case CD_QUAD:
            return format(".quad %d", code->ival);

        case CD_ASCII:
            return format(".ascii \"%s\"",
                          escape_string(code->sval, code->ival));
    }
    warn(format("code.c %d", code->kind));
    assert(0);
}

void dump_code(Code *code, FILE *fh)
{
    char *str = code2str(code);
    if (str != NULL) fprintf(fh, "%s\n", str);
}

static Code *value(int value) { return new_value_code(value); }

static Code *addrof_label(Code *reg, char *label)
{
    return new_addrof_label_code(reg, label);
}

static Code *addrof(Code *reg, int offset)
{
    return new_addrof_code(reg, offset);
}

static int temp_reg_table;

static void init_temp_reg() { temp_reg_table = 0; }

static int get_temp_reg()
{
    for (int i = 0; i < 6; i++) {
        if (temp_reg_table & (1 << i)) continue;
        temp_reg_table |= (1 << i);
        return i + 7;  // corresponding to reg_name's index
    }

    error("no more register");
}

static void restore_temp_reg(int i) { temp_reg_table &= ~(1 << (i - 7)); }

static const char *reg_name(int byte, int i)
{
    const char *lreg[13];
    lreg[0] = "%al";
    lreg[1] = "%dil";
    lreg[2] = "%sil";
    lreg[3] = "%dl";
    lreg[4] = "%cl";
    lreg[5] = "%r8b";
    lreg[6] = "%r9b";
    lreg[7] = "%r10b";
    lreg[8] = "%r11b";
    lreg[9] = "%r12b";
    lreg[10] = "%r13b";
    lreg[11] = "%r14b";
    lreg[12] = "%r15b";
    const char *xreg[13];
    xreg[0] = "%ax";
    xreg[1] = "%di";
    xreg[2] = "%si";
    xreg[3] = "%dx";
    xreg[4] = "%cx";
    xreg[5] = "%r8w";
    xreg[6] = "%r9w";
    xreg[7] = "%r10w";
    xreg[8] = "%r11w";
    xreg[9] = "%r12w";
    xreg[10] = "%r13w";
    xreg[11] = "%r14w";
    xreg[12] = "%r15w";
    const char *ereg[13];
    ereg[0] = "%eax";
    ereg[1] = "%edi";
    ereg[2] = "%esi";
    ereg[3] = "%edx";
    ereg[4] = "%ecx";
    ereg[5] = "%r8d";
    ereg[6] = "%r9d";
    ereg[7] = "%r10d";
    ereg[8] = "%r11d";
    ereg[9] = "%r12d";
    ereg[10] = "%r13d";
    ereg[11] = "%r14d";
    ereg[12] = "%r15d";
    const char *rreg[13];
    rreg[0] = "%rax";
    rreg[1] = "%rdi";
    rreg[2] = "%rsi";
    rreg[3] = "%rdx";
    rreg[4] = "%rcx";
    rreg[5] = "%r8";
    rreg[6] = "%r9";
    rreg[7] = "%r10";
    rreg[8] = "%r11";
    rreg[9] = "%r12";
    rreg[10] = "%r13";
    rreg[11] = "%r14";
    rreg[12] = "%r15";

    assert(0 <= i && i <= 12);

    switch (byte) {
        case 1:
            return lreg[i];
        case 2:
            return xreg[i];
        case 4:
            return ereg[i];
        case 8:
            return rreg[i];
        default:
            assert(0);
    }
}

static int lookup_member_offset(Vector *members, char *member)
{
    // O(n)
    int size = vector_size(members);
    for (int i = 0; i < size; i++) {
        StructMember *sm = (StructMember *)vector_get(members, i);
        if ((sm->type->kind == TY_STRUCT || sm->type->kind == TY_UNION) &&
            sm->name == NULL) {
            // nested anonymous struct with no varname.
            // its members can be accessed like parent struct's members.
            int offset = lookup_member_offset(sm->type->members, member);
            if (offset >= 0) return sm->offset + offset;
        }
        else if (strcmp(sm->name, member) == 0) {
            return sm->offset;
        }
    }
    return -1;
}

typedef struct {
    char *continue_label, *break_label;
    int reg_save_area_stack_idx, overflow_arg_area_stack_idx;
    Vector *code;
} CodeEnv;
CodeEnv *codeenv;

#define SAVE_BREAK_CXT char *break_cxt__org_break_label = codeenv->break_label;
#define RESTORE_BREAK_CXT codeenv->break_label = break_cxt__org_break_label;
#define SAVE_CONTINUE_CXT \
    char *continue_cxt__org_continue_label = codeenv->continue_label;
#define RESTORE_CONTINUE_CXT \
    codeenv->continue_label = continue_cxt__org_continue_label;
#define SAVE_VARIADIC_CXT                                \
    int save_variadic_cxt__reg_save_area_stack_idx =     \
            codeenv->reg_save_area_stack_idx,            \
        save_variadic_cxt__overflow_arg_area_stack_idx = \
            codeenv->overflow_arg_area_stack_idx;

#define RESTORE_VARIADIC_CXT                        \
    codeenv->reg_save_area_stack_idx =              \
        save_variadic_cxt__reg_save_area_stack_idx; \
    codeenv->overflow_arg_area_stack_idx =          \
        save_variadic_cxt__overflow_arg_area_stack_idx;

static void init_code_env()
{
    codeenv = (CodeEnv *)safe_malloc(sizeof(CodeEnv));
    codeenv->continue_label = codeenv->break_label = NULL;
    codeenv->reg_save_area_stack_idx = 0;
    codeenv->code = new_vector();
}

static void appcode(Code *code) { vector_push_back(codeenv->code, code); }

static void appcomment(char *str)
{
    Code *code = new_code(CD_COMMENT);
    code->sval = str;
    appcode(code);
}

static Code *last_appended_code()
{
    for (int i = vector_size(codeenv->code) - 1; i >= 0; i--) {
        Code *str = vector_get(codeenv->code, i);
        if (str) return str;
    }

    assert(0);
}

static void add_read_dep(Code *dep)
{
    vector_push_back(last_appended_code()->read_dep, dep);
}

static void disable_elimination()
{
    last_appended_code()->can_be_eliminated = 0;
}

static int nbyte_of_reg(int reg)
{
    int ret = (reg & (REG_8 | REG_16 | REG_32 | REG_64)) >> 5;
    assert(ret == 1 || ret == 2 || ret == 3 || ret == 4);
    return ret;
}

static Type *x86_64_analyze_type(Type *type)
{
    if (type == NULL) return NULL;

    switch (type->kind) {
        case TY_INT:
            type->nbytes = 4;
            break;

        case TY_CHAR:
            type->nbytes = 1;
            break;

        case TY_PTR:
            type->ptr_of = x86_64_analyze_type(type->ptr_of);
            type->nbytes = 8;
            break;

        case TY_ARY:
            type->ary_of = x86_64_analyze_type(type->ary_of);
            type->nbytes = type->len * type->ary_of->nbytes;
            break;

        case TY_STRUCT: {
            if (type->members == NULL) break;

            int offset = 0;
            for (int i = 0; i < vector_size(type->members); i++) {
                StructMember *member =
                    (StructMember *)vector_get(type->members, i);
                member->type = x86_64_analyze_type(member->type);

                // calc offset
                offset = roundup(offset, alignment_of(member->type));
                member->offset = offset;
                offset += member->type->nbytes;
            }
            type->nbytes = roundup(offset, alignment_of(type));
        } break;

        case TY_UNION: {
            if (type->members == NULL) break;

            int max_nbytes = 0;
            for (int i = 0; i < vector_size(type->members); i++) {
                StructMember *member =
                    (StructMember *)vector_get(type->members, i);
                member->type = x86_64_analyze_type(member->type);

                // offset is always zero.
                member->offset = 0;
                max_nbytes = max(max_nbytes, member->type->nbytes);
            }
            type->nbytes = roundup(max_nbytes, alignment_of(type));
        } break;

        case TY_ENUM:
            break;

        case TY_VOID:
            break;

        default:
            assert(0);
    }

    return type;
}

static int x86_64_eval_ast_int(AST *ast)
{
    assert(ast != NULL);

    switch (ast->kind) {
        case AST_INT:
            return ast->ival;

        case AST_SIZEOF:
            return ast->lhs->type->nbytes;

        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        case AST_REM:
        case AST_LSHIFT:
        case AST_RSHIFT:
        case AST_LT:
        case AST_LTE:
        case AST_EQ:
        case AST_AND:
        case AST_XOR:
        case AST_OR:
        case AST_LAND:
        case AST_LOR:
        case AST_NEQ: {
            int lhs = x86_64_eval_ast_int(ast->lhs),
                rhs = x86_64_eval_ast_int(ast->rhs), ret = 0;

            switch (ast->kind) {
                case AST_ADD:
                    ret = lhs + rhs;
                    break;
                case AST_SUB:
                    ret = lhs - rhs;
                    break;
                case AST_MUL:
                    ret = lhs * rhs;
                    break;
                case AST_DIV:
                    ret = lhs / rhs;
                    break;
                case AST_REM:
                    ret = lhs % rhs;
                    break;
                case AST_LSHIFT:
                    ret = lhs << rhs;
                    break;
                case AST_RSHIFT:
                    ret = lhs >> rhs;
                    break;
                case AST_LT:
                    ret = lhs < rhs;
                    break;
                case AST_LTE:
                    ret = lhs <= rhs;
                    break;
                case AST_EQ:
                    ret = lhs == rhs;
                    break;
                case AST_AND:
                    ret = lhs & rhs;
                    break;
                case AST_XOR:
                    ret = lhs ^ rhs;
                    break;
                case AST_OR:
                    ret = lhs | rhs;
                    break;
                case AST_LAND:
                    ret = lhs && rhs;
                    break;
                case AST_LOR:
                    ret = lhs || rhs;
                    break;
                case AST_NEQ:
                    ret = lhs != rhs;
                    break;
                default:
                    assert(0);
            }
            return ret;
        }

        case AST_COMPL:
        case AST_UNARY_MINUS:
        case AST_NOT:
        case AST_CAST:
        case AST_CONSTANT: {
            int lhs = x86_64_eval_ast_int(ast->lhs), ret = 0;
            switch (ast->kind) {
                case AST_COMPL:
                    ret = ~lhs;
                    break;
                case AST_UNARY_MINUS:
                    ret = -lhs;
                    break;
                case AST_NOT:
                    ret = !lhs;
                    break;
                case AST_CAST:
                case AST_CONSTANT:
                    ret = lhs;
                    break;
                default:
                    assert(0);
            }
            return ret;
        }

        case AST_COND: {
            int cond = x86_64_eval_ast_int(ast->cond),
                then = x86_64_eval_ast_int(ast->then),
                els = x86_64_eval_ast_int(ast->els);
            return cond ? then : els;
        }

        default:
            assert(0);
    }
}

// all Type* should pass x86_64_analyze_type().
// all AST* should pass x86_64_analyze_ast_detail()
static AST *x86_64_analyze_ast_detail(AST *ast)
{
    if (ast == NULL) return NULL;

    switch (ast->kind) {
        case AST_INT:
        case AST_LVAR:
        case AST_GVAR:
            ast->type = x86_64_analyze_type(ast->type);
            break;

        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        case AST_REM:
        case AST_LSHIFT:
        case AST_RSHIFT:
        case AST_LT:
        case AST_LTE:
        case AST_EQ:
        case AST_AND:
        case AST_XOR:
        case AST_OR:
        case AST_LAND:
        case AST_LOR:
        case AST_ASSIGN:
        case AST_VA_START:
            ast->type = x86_64_analyze_type(ast->type);
            ast->lhs = x86_64_analyze_ast_detail(ast->lhs);
            ast->rhs = x86_64_analyze_ast_detail(ast->rhs);
            break;

        case AST_COMPL:
        case AST_UNARY_MINUS:
        case AST_PREINC:
        case AST_POSTINC:
        case AST_PREDEC:
        case AST_POSTDEC:
        case AST_ADDR:
        case AST_INDIR:
        case AST_CAST:
        case AST_CHAR2INT:
        case AST_LVALUE2RVALUE:
        case AST_VA_ARG_INT:
        case AST_VA_ARG_CHARP:
            ast->type = x86_64_analyze_type(ast->type);
            ast->lhs = x86_64_analyze_ast_detail(ast->lhs);
            break;

        case AST_ARY2PTR:
            ast->type = x86_64_analyze_type(ast->type);
            ast->ary = x86_64_analyze_ast_detail(ast->ary);
            break;

        case AST_EXPR_LIST:
            ast->type = x86_64_analyze_type(ast->type);

            for (int i = 0; i < vector_size(ast->exprs); i++) {
                AST *expr = (AST *)vector_get(ast->exprs, i);
                vector_set(ast->exprs, i, x86_64_analyze_ast_detail(expr));
            }
            break;

        case AST_COND:
            ast->type = x86_64_analyze_type(ast->type);
            ast->cond = x86_64_analyze_ast_detail(ast->cond);
            ast->then = x86_64_analyze_ast_detail(ast->then);
            ast->els = x86_64_analyze_ast_detail(ast->els);
            break;

        case AST_FUNCCALL:
            ast->type = x86_64_analyze_type(ast->type);
            for (int i = 0; i < vector_size(ast->args); i++) {
                AST *arg = (AST *)vector_get(ast->args, i);
                vector_set(ast->args, i, x86_64_analyze_ast_detail(arg));
            }
            break;

        case AST_IF:
            ast->cond = x86_64_analyze_ast_detail(ast->cond);
            ast->then = x86_64_analyze_ast_detail(ast->then);
            ast->els = x86_64_analyze_ast_detail(ast->els);
            break;

        case AST_SWITCH:
            ast->target = x86_64_analyze_ast_detail(ast->target);
            ast->switch_body = x86_64_analyze_ast_detail(ast->switch_body);
            for (int i = 0; i < vector_size(ast->cases); i++) {
                SwitchCase *cas = (SwitchCase *)vector_get(ast->cases, i);
                cas->cond = x86_64_analyze_ast_detail(cas->cond);
                vector_set(ast->cases, i, cas);
            }
            break;

        case AST_FOR:
            ast->initer = x86_64_analyze_ast_detail(ast->initer);
            ast->midcond = x86_64_analyze_ast_detail(ast->midcond);
            ast->iterer = x86_64_analyze_ast_detail(ast->iterer);
            ast->for_body = x86_64_analyze_ast_detail(ast->for_body);
            break;

        case AST_DOWHILE:
            ast->cond = x86_64_analyze_ast_detail(ast->cond);
            ast->then = x86_64_analyze_ast_detail(ast->then);
            break;

        case AST_RETURN:
        case AST_EXPR_STMT:
            ast->lhs = x86_64_analyze_ast_detail(ast->lhs);
            break;

        case AST_MEMBER_REF:
            ast->type = x86_64_analyze_type(ast->type);
            ast->stsrc = x86_64_analyze_ast_detail(ast->stsrc);
            break;

        case AST_COMPOUND:
            for (int i = 0; i < vector_size(ast->stmts); i++) {
                AST *stmt = (AST *)vector_get(ast->stmts, i);
                vector_set(ast->stmts, i, x86_64_analyze_ast_detail(stmt));
            }
            break;

        case AST_LVAR_DECL_INIT:
        case AST_GVAR_DECL_INIT:
            ast->lhs = x86_64_analyze_ast_detail(ast->lhs);
            ast->rhs = x86_64_analyze_ast_detail(ast->rhs);
            break;

        case AST_DECL_LIST:
            for (int i = 0; i < vector_size(ast->decls); i++) {
                AST *decl = (AST *)vector_get(ast->decls, i);
                vector_set(ast->decls, i, x86_64_analyze_ast_detail(decl));
            }
            break;

        case AST_FUNCDEF:
            ast->type = x86_64_analyze_type(ast->type);
            if (ast->params) {
                for (int i = 0; i < vector_size(ast->params); i++) {
                    AST *param = (AST *)vector_get(ast->params, i);
                    param->type = x86_64_analyze_type(param->type);
                }
            }
            ast->body = x86_64_analyze_ast_detail(ast->body);
            break;

        case AST_NOP:
        case AST_GVAR_DECL:
        case AST_LVAR_DECL:
        case AST_FUNC_DECL:
        case AST_GOTO:
        case AST_BREAK:
        case AST_CONTINUE:
            break;

        case AST_LABEL:
            ast->label_stmt = x86_64_analyze_ast_detail(ast->label_stmt);
            break;

        case AST_SIZEOF:
            if (ast->lhs->kind == AST_NOP)
                ast->lhs->type = x86_64_analyze_type(ast->lhs->type);
            else
                ast->lhs = x86_64_analyze_ast_detail(ast->lhs);
            ast->type = x86_64_analyze_type(ast->type);
            break;

        case AST_CONSTANT:
            assert(ast->type->kind == TY_INT);
            ast->lhs = x86_64_analyze_ast_detail(ast->lhs);
            ast = new_int_ast(x86_64_eval_ast_int(ast));
            break;

        default:
            assert(0);
    }

    return ast;
}

static void x86_64_analyze_ast(Vector *asts)
{
    int nasts = vector_size(asts);
    for (int i = 0; i < nasts; i++) {
        AST *ast = (AST *)vector_get(asts, i);
        vector_set(asts, i, x86_64_analyze_ast_detail(ast));
    }
}

static void generate_basic_block_start_maker()
{
    appcomment("BLOCK START");
    appcode(new_code(MRK_BASIC_BLOCK_START));
}

static void generate_basic_block_end_marker()
{
    appcode(new_code(MRK_BASIC_BLOCK_END));
    appcomment("BLOCK END");
}

static void generate_funcdef_start_marker()
{
    appcomment("FUNCDEF START");
    appcode(new_code(MRK_FUNCDEF_START));
}

static void generate_funcdef_end_marker()
{
    appcode(new_code(MRK_FUNCDEF_END));
    appcomment("FUNCDEF END");
}

static void generate_funcdef_return_marker()
{
    appcode(new_code(MRK_FUNCDEF_RETURN));
}

static void generate_mov_mem_reg(int nbyte, int src_reg, int dst_reg)
{
    switch (nbyte) {
        case 1: {
            appcode(MOVSBL(addrof(nbyte_reg(8, src_reg), 0),
                           new_code(reg_of_nbyte(4, dst_reg))));
        } break;
        case 4:
        case 8:
            appcode(MOV(addrof(nbyte_reg(8, src_reg), 0),
                        new_code(reg_of_nbyte(nbyte, dst_reg))));
            break;
        default:
            assert(0);
    }
}

static int x86_64_generate_code_detail(AST *ast)
{
    switch (ast->kind) {
        case AST_INT: {
            int reg = get_temp_reg();
            appcode(MOV(value(ast->ival), nbyte_reg(4, reg)));
            return reg;
        }

        case AST_RETURN: {
            assert(temp_reg_table == 0);
            if (ast->lhs) {
                int reg = x86_64_generate_code_detail(ast->lhs);
                restore_temp_reg(reg);
                appcode(MOV(nbyte_reg(8, reg), RAX()));
            }
            else {
                appcode(MOV(value(0), RAX()));
            }
            assert(temp_reg_table == 0);
            generate_funcdef_return_marker();
            appcode(MOV(RBP(), RSP()));
            appcode(POP(RBP()));
            appcode(RET());
            return -1;
        }

        case AST_ADD: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);

            // int + ptr
            // TODO: shift
            // TODO: long
            if (match_type2(ast->lhs, ast->rhs, TY_INT, TY_PTR)) {
                appcode(MOVSLQ(nbyte_reg(4, lreg), nbyte_reg(8, lreg)));
                appcode(CLTQ());
                appcode(IMUL(value(ast->rhs->type->ptr_of->nbytes),
                             nbyte_reg(8, lreg)));
            }

            int nbytes = ast->type->nbytes;
            appcode(ADD(nbyte_reg(nbytes, lreg), nbyte_reg(nbytes, rreg)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_SUB: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);

            // ptr - int
            // TODO: shift
            // TODO: long
            if (match_type2(ast->lhs, ast->rhs, TY_PTR, TY_INT)) {
                appcode(MOVSLQ(nbyte_reg(4, rreg), nbyte_reg(8, rreg)));
                appcode(CLTQ());
                appcode(IMUL(value(ast->lhs->type->ptr_of->nbytes),
                             nbyte_reg(8, rreg)));
            }

            // ptr - ptr
            if (match_type2(ast->lhs, ast->rhs, TY_PTR, TY_PTR)) {
                // assume pointer size is 8.
                appcode(SUB(nbyte_reg(8, rreg), nbyte_reg(8, lreg)));
                switch (ast->lhs->type->ptr_of->nbytes) {
                    case 1:
                        break;
                    case 4:
                        appcode(SAR(value(2), nbyte_reg(8, lreg)));
                        break;
                    case 8:
                        appcode(SAR(value(3), nbyte_reg(8, lreg)));
                        break;
                    default:
                        assert(0);  // TODO: idiv?
                }
            }
            else {
                int nbytes = ast->type->nbytes;
                appcode(SUB(nbyte_reg(nbytes, rreg), nbyte_reg(nbytes, lreg)));
            }

            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_MUL: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);
            int nbytes = 4;  // TODO: long
            appcode(IMUL(nbyte_reg(nbytes, lreg), nbyte_reg(nbytes, rreg)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_DIV: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);
            appcode(MOV(nbyte_reg(ast->type->nbytes, lreg),
                        nbyte_reg(ast->type->nbytes, 0)));
            appcode(CLTD());  // TODO: assume EAX
            appcode(IDIV(nbyte_reg(ast->type->nbytes, rreg)));
            add_read_dep(RAX());
            appcode(MOV(RAX(), nbyte_reg(8, rreg)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_REM: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);
            appcode(MOV(nbyte_reg(ast->type->nbytes, lreg),
                        nbyte_reg(ast->type->nbytes, 0)));
            appcode(CLTD());  // TODO: assume EAX
            appcode(IDIV(nbyte_reg(ast->type->nbytes, rreg)));
            add_read_dep(RAX());
            appcode(MOV(RDX(), nbyte_reg(8, rreg)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_UNARY_MINUS: {
            int reg = x86_64_generate_code_detail(ast->lhs);
            appcode(NEG(nbyte_reg(ast->type->nbytes, reg)));
            return reg;
        }

        case AST_COMPL: {
            int reg = x86_64_generate_code_detail(ast->lhs);
            appcode(NOT(nbyte_reg(ast->type->nbytes, reg)));
            return reg;
        }

        case AST_LSHIFT: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);
            appcode(MOV(nbyte_reg(1, rreg), CL()));
            appcode(SAL(CL(), nbyte_reg(ast->type->nbytes, lreg)));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_RSHIFT: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);
            appcode(MOV(nbyte_reg(1, rreg), CL()));
            appcode(SAR(CL(), nbyte_reg(ast->type->nbytes, lreg)));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_LT: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);
            appcode(CMP(nbyte_reg(ast->type->nbytes, rreg),
                        nbyte_reg(ast->type->nbytes, lreg)));
            appcode(SETL(AL()));
            appcode(MOVZB(AL(), nbyte_reg(ast->type->nbytes, lreg)));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_LTE: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);
            appcode(CMP(nbyte_reg(ast->type->nbytes, rreg),
                        nbyte_reg(ast->type->nbytes, lreg)));
            appcode(SETLE(AL()));
            appcode(MOVZB(AL(), nbyte_reg(ast->type->nbytes, lreg)));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_EQ: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);
            appcode(CMP(nbyte_reg(ast->type->nbytes, rreg),
                        nbyte_reg(ast->type->nbytes, lreg)));
            appcode(SETE(AL()));
            appcode(MOVZB(AL(), nbyte_reg(ast->type->nbytes, lreg)));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_AND: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);
            appcode(AND(nbyte_reg(ast->type->nbytes, lreg),
                        nbyte_reg(ast->type->nbytes, rreg)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_XOR: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);
            appcode(XOR(nbyte_reg(ast->type->nbytes, lreg),
                        nbyte_reg(ast->type->nbytes, rreg)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_OR: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);
            appcode(OR(nbyte_reg(ast->type->nbytes, lreg),
                       nbyte_reg(ast->type->nbytes, rreg)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_LAND: {
            char *false_label = make_label_string(),
                 *exit_label = make_label_string(),
                 *beyond_label0 = make_label_string(),
                 *beyond_label1 = make_label_string();
            int lreg = x86_64_generate_code_detail(ast->lhs);
            restore_temp_reg(lreg);
            appcode(CMP(value(0), nbyte_reg(ast->type->nbytes, lreg)));

            // JE(false_label)
            appcode(JNE(beyond_label0));
            appcode(JMP(false_label));
            appcode(LABEL(beyond_label0));

            // don't execute rhs expression if lhs is false.
            int rreg = x86_64_generate_code_detail(ast->rhs);
            Code *rreg_code = nbyte_reg(ast->type->nbytes, rreg);
            appcode(CMP(value(0), rreg_code));

            // JE(false_label)
            appcode(JNE(beyond_label1));
            appcode(JMP(false_label));
            appcode(LABEL(beyond_label1));

            appcode(MOV(value(1), rreg_code));
            appcode(JMP(exit_label));
            appcode(LABEL(false_label));
            appcode(MOV(value(0), rreg_code));
            appcode(LABEL(exit_label));
            return rreg;
        }

        case AST_LOR: {
            char *true_label = make_label_string(),
                 *exit_label = make_label_string(),
                 *beyond_label0 = make_label_string(),
                 *beyond_label1 = make_label_string();
            int lreg = x86_64_generate_code_detail(ast->lhs);
            restore_temp_reg(lreg);
            appcode(CMP(value(0), nbyte_reg(ast->type->nbytes, lreg)));

            // JNE(true_label);
            appcode(JE(beyond_label0));
            appcode(JMP(true_label));
            appcode(LABEL(beyond_label0));

            // don't execute rhs expression if lhs is true.
            int rreg = x86_64_generate_code_detail(ast->rhs);
            Code *rreg_code = nbyte_reg(ast->type->nbytes, rreg);
            appcode(CMP(value(0), rreg_code));

            // JNE(true_label);
            appcode(JE(beyond_label1));
            appcode(JMP(true_label));
            appcode(LABEL(beyond_label1));

            appcode(CMP(value(0), rreg_code));
            appcode(JMP(exit_label));
            appcode(LABEL(true_label));
            appcode(MOV(value(1), rreg_code));
            appcode(LABEL(exit_label));
            return rreg;
        }

        case AST_FUNCDEF: {
            // allocate stack
            int stack_idx = 0;
            for (int i = ast->params ? max(0, vector_size(ast->params) - 6) : 0;
                 i < vector_size(ast->env->scoped_vars); i++) {
                AST *var = (AST *)(vector_get(ast->env->scoped_vars, i));

                stack_idx -= var->type->nbytes;
                var->stack_idx = stack_idx;
            }

            stack_idx -= (!!ast->is_variadic) * 48;

            // generate code
            if (!ast->type->is_static) appcode(GLOBAL(ast->fname));
            appcode(LABEL(ast->fname));
            appcode(PUSH(RBP()));
            appcode(MOV(RSP(), RBP()));
            int needed_stack_size = roundup(-stack_idx, 16);
            if (needed_stack_size > 0)
                appcode(SUB(value(needed_stack_size), RSP()));
            generate_funcdef_start_marker();

            // assign param to localvar
            if (ast->params) {
                for (int i = 0; i < vector_size(ast->params); i++) {
                    AST *var = lookup_var(
                        ast->env, ((AST *)vector_get(ast->params, i))->varname);
                    if (i < 6)
                        appcode(MOV(nbyte_reg(var->type->nbytes, i + 1),
                                    addrof(RBP(), var->stack_idx)));
                    else
                        // should avoid return pointer and saved %rbp
                        var->stack_idx = 16 + (i - 6) * 8;
                }
            }

            // place Register Save Area if the function has variadic params.
            if (ast->is_variadic)
                for (int i = 0; i < 6; i++)
                    appcode(MOV(nbyte_reg(8, i + 1),
                                addrof(RBP(), stack_idx + i * 8)));

            // generate body
            SAVE_VARIADIC_CXT;
            codeenv->reg_save_area_stack_idx = stack_idx;
            codeenv->overflow_arg_area_stack_idx =
                ast->params ? max(0, vector_size(ast->params) - 6) * 8 + 16
                            : -1;
            assert(temp_reg_table == 0);
            x86_64_generate_code_detail(ast->body);
            assert(temp_reg_table == 0);
            RESTORE_VARIADIC_CXT;

            if (ast->type->kind != TY_VOID) appcode(MOV(value(0), RAX()));
            generate_funcdef_end_marker();
            appcode(MOV(RBP(), RSP()));
            appcode(POP(RBP()));
            appcode(RET());
            return -1;
        }

        case AST_ASSIGN: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = x86_64_generate_code_detail(ast->rhs);

            appcode(MOV(nbyte_reg(ast->type->nbytes, rreg),
                        addrof(nbyte_reg(8, lreg), 0)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_EXPR_LIST: {
            int reg;
            for (int i = 0; i < vector_size(ast->exprs); i++) {
                reg = x86_64_generate_code_detail(
                    (AST *)vector_get(ast->exprs, i));
                if (i != vector_size(ast->exprs) - 1) restore_temp_reg(reg);
            }
            return reg;
        }

        case AST_LVAR: {
            int reg = get_temp_reg();
            Code *reg_code = nbyte_reg(8, reg);
            if (ast->type->is_static || ast->type->is_extern)
                appcode(LEA(addrof_label(RIP(), ast->gen_varname), reg_code));
            else
                appcode(LEA(addrof(RBP(), ast->stack_idx), reg_code));
            return reg;
        }

        case AST_GVAR: {
            int reg = get_temp_reg();
            Code *reg_code = nbyte_reg(8, reg);
            appcode(LEA(addrof_label(RIP(), ast->gen_varname), reg_code));
            return reg;
        }

        case AST_FUNCCALL: {
            appcode(PUSH(R10()));
            appcode(PUSH(R11()));
            for (int i = vector_size(ast->args) - 1; i >= 0; i--) {
                int reg = x86_64_generate_code_detail(
                    (AST *)(vector_get(ast->args, i)));
                appcode(PUSH(nbyte_reg(8, reg)));
                restore_temp_reg(reg);
            }
            for (int i = 0; i < min(6, vector_size(ast->args)); i++)
                appcode(POP(nbyte_reg(8, i + 1)));
            appcode(MOV(value(0), EAX()));
            disable_elimination();

            {
                Code *code = new_code(INST_CALL);
                code->label = ast->fname;
                appcode(code);
            }
            appcode(ADD(value(8 * max(0, vector_size(ast->args) - 6)), RSP()));
            appcode(POP(R11()));
            appcode(POP(R10()));
            int reg = get_temp_reg();
            appcode(MOV(RAX(), nbyte_reg(8, reg)));
            return reg;
        }

        case AST_POSTINC: {
            int lreg = x86_64_generate_code_detail(ast->lhs);
            Code *lreg_code = nbyte_reg(8, lreg);
            int reg = get_temp_reg();
            generate_mov_mem_reg(ast->type->nbytes, lreg, reg);

            if (match_type(ast->lhs, TY_PTR)) {
                appcode(ADDQ(value(ast->lhs->type->ptr_of->nbytes),
                             addrof(lreg_code, 0)));
            }
            else {
                switch (ast->lhs->type->nbytes) {
                    case 4:
                        appcode(INCL(addrof(lreg_code, 0)));
                        break;
                    case 8:
                        appcode(INCQ(addrof(lreg_code, 0)));
                        break;
                    default:
                        assert(0);
                }
            }

            restore_temp_reg(lreg);
            return reg;
        }

        case AST_PREINC: {
            int lreg = x86_64_generate_code_detail(ast->lhs);
            Code *lreg_code = nbyte_reg(8, lreg);
            int reg = get_temp_reg();

            if (match_type(ast->lhs, TY_PTR)) {
                appcode(ADDQ(value(ast->lhs->type->ptr_of->nbytes),
                             addrof(lreg_code, 0)));
            }
            else {
                switch (ast->lhs->type->nbytes) {
                    case 4:
                        appcode(INCL(addrof(lreg_code, 0)));
                        break;
                    case 8:
                        appcode(INCQ(addrof(lreg_code, 0)));
                        break;
                    default:
                        assert(0);
                }
            }

            generate_mov_mem_reg(ast->type->nbytes, lreg, reg);

            restore_temp_reg(lreg);
            return reg;
        }

        case AST_POSTDEC: {
            int lreg = x86_64_generate_code_detail(ast->lhs);
            Code *lreg_code = nbyte_reg(8, lreg);
            int reg = get_temp_reg();
            generate_mov_mem_reg(ast->type->nbytes, lreg, reg);

            if (match_type(ast->lhs, TY_PTR)) {
                appcode(ADDQ(value(-ast->lhs->type->ptr_of->nbytes),
                             addrof(lreg_code, 0)));
            }
            else {
                switch (ast->lhs->type->nbytes) {
                    case 4:
                        appcode(DECL(addrof(lreg_code, 0)));
                        break;
                    case 8:
                        appcode(DECQ(addrof(lreg_code, 0)));
                        break;
                    default:
                        assert(0);
                }
            }

            restore_temp_reg(lreg);
            return reg;
        }

        case AST_PREDEC: {
            int lreg = x86_64_generate_code_detail(ast->lhs);
            Code *lreg_code = nbyte_reg(8, lreg);
            int reg = get_temp_reg();

            if (match_type(ast->lhs, TY_PTR)) {
                appcode(ADDQ(value(-ast->lhs->type->ptr_of->nbytes),
                             addrof(lreg_code, 0)));
            }
            else {
                switch (ast->lhs->type->nbytes) {
                    case 4:
                        appcode(DECL(addrof(lreg_code, 0)));
                        break;
                    case 8:
                        appcode(DECQ(addrof(lreg_code, 0)));
                        break;
                    default:
                        assert(0);
                }
            }

            generate_mov_mem_reg(ast->type->nbytes, lreg, reg);

            restore_temp_reg(lreg);
            return reg;
        }

        case AST_ADDR:
        case AST_INDIR:
        case AST_CAST:
            return x86_64_generate_code_detail(ast->lhs);

        case AST_COND: {
            char *false_label = make_label_string(),
                 *exit_label = make_label_string(),
                 *beyond_label = make_label_string();

            int cond_reg = x86_64_generate_code_detail(ast->cond);
            restore_temp_reg(cond_reg);
            appcode(MOV(nbyte_reg(8, cond_reg), RAX()));
            appcode(CMP(value(0), EAX()));

            // JE(false_label);
            appcode(JNE(beyond_label));
            appcode(JMP(false_label));
            appcode(LABEL(beyond_label));

            int then_reg = x86_64_generate_code_detail(ast->then);
            restore_temp_reg(then_reg);
            appcode(PUSH(nbyte_reg(8, then_reg)));
            appcode(JMP(exit_label));
            appcode(LABEL(false_label));
            if (ast->els != NULL) {
                int els_reg = x86_64_generate_code_detail(ast->els);
                restore_temp_reg(els_reg);
                appcode(PUSH(nbyte_reg(8, els_reg)));
            }
            appcode(LABEL(exit_label));
            int reg = get_temp_reg();
            appcode(POP(nbyte_reg(8, reg)));
            return reg;
        }

        case AST_IF: {
            char *false_label = make_label_string(),
                 *exit_label = make_label_string(),
                 *beyond_label = make_label_string();

            int cond_reg = x86_64_generate_code_detail(ast->cond);
            restore_temp_reg(cond_reg);
            appcode(MOV(nbyte_reg(8, cond_reg), RAX()));
            appcode(CMP(value(0), EAX()));

            // JE(false_label)
            appcode(JNE(beyond_label));
            appcode(JMP(false_label));
            appcode(LABEL(beyond_label));

            x86_64_generate_code_detail(ast->then);
            appcode(JMP(exit_label));
            appcode(LABEL(false_label));
            if (ast->els != NULL) x86_64_generate_code_detail(ast->els);
            appcode(LABEL(exit_label));
            return -1;
        }

        case AST_SWITCH: {
            int target_reg = x86_64_generate_code_detail(ast->target);
            restore_temp_reg(target_reg);

            for (int i = 0; i < vector_size(ast->cases); i++) {
                SwitchCase *cas = (SwitchCase *)vector_get(ast->cases, i);
                assert(cas->cond->kind == AST_INT);
                appcode(CMP(value(cas->cond->ival),
                            nbyte_reg(ast->target->type->nbytes, target_reg)));
                // case has been already labeled when analyzing.
                // JE(cas->label_name)
                char *beyond_label = make_label_string();
                appcode(JNE(beyond_label));
                appcode(JMP(cas->label_name));
                appcode(LABEL(beyond_label));
            }
            char *exit_label = make_label_string();
            if (ast->default_label)
                appcode(JMP(ast->default_label));
            else
                appcode(JMP(exit_label));

            SAVE_BREAK_CXT;
            codeenv->break_label = exit_label;
            x86_64_generate_code_detail(ast->switch_body);
            appcode(LABEL(exit_label));
            RESTORE_BREAK_CXT;

            return -1;
        }

        case AST_DOWHILE: {
            SAVE_BREAK_CXT;
            SAVE_CONTINUE_CXT;
            codeenv->break_label = make_label_string();
            codeenv->continue_label = make_label_string();
            char *start_label = make_label_string();

            appcode(LABEL(start_label));
            x86_64_generate_code_detail(ast->then);
            appcode(LABEL(codeenv->continue_label));
            int cond_reg = x86_64_generate_code_detail(ast->cond);
            appcode(
                CMP(value(0), nbyte_reg(ast->cond->type->nbytes, cond_reg)));
            // JNE(start_label)
            char *beyond_label = make_label_string();
            appcode(JE(beyond_label));
            appcode(JMP(start_label));
            appcode(LABEL(beyond_label));

            appcode(LABEL(codeenv->break_label));

            restore_temp_reg(cond_reg);
            RESTORE_BREAK_CXT;
            RESTORE_CONTINUE_CXT;

            return -1;
        }

        case AST_FOR: {
            SAVE_BREAK_CXT;
            SAVE_CONTINUE_CXT;
            codeenv->break_label = make_label_string();
            codeenv->continue_label = make_label_string();
            char *start_label = make_label_string();

            if (ast->initer != NULL) {
                int reg = x86_64_generate_code_detail(ast->initer);
                if (reg != -1) restore_temp_reg(reg);  // if expr
            }
            appcode(LABEL(start_label));
            if (ast->midcond != NULL) {
                int reg = x86_64_generate_code_detail(ast->midcond);
                appcode(
                    CMP(value(0), nbyte_reg(ast->midcond->type->nbytes, reg)));
                // JE(codeenv->break_label)
                char *beyond_label = make_label_string();
                appcode(JNE(beyond_label));
                appcode(JMP(codeenv->break_label));
                appcode(LABEL(beyond_label));
                restore_temp_reg(reg);
            }
            x86_64_generate_code_detail(ast->for_body);
            appcode(LABEL(codeenv->continue_label));
            if (ast->iterer != NULL) {
                int reg = x86_64_generate_code_detail(ast->iterer);
                if (reg != -1) restore_temp_reg(reg);  // if nop
            }
            appcode(JMP(start_label));
            appcode(LABEL(codeenv->break_label));

            RESTORE_BREAK_CXT;
            RESTORE_CONTINUE_CXT;

            return -1;
        }

        case AST_LABEL:
            appcode(LABEL(ast->label_name));
            x86_64_generate_code_detail(ast->label_stmt);
            return -1;

        case AST_BREAK:
            appcode(JMP(codeenv->break_label));
            return -1;

        case AST_CONTINUE:
            appcode(JMP(codeenv->continue_label));
            return -1;

        case AST_GOTO:
            appcode(JMP(ast->label_name));
            return -1;

        case AST_MEMBER_REF: {
            int offset =
                lookup_member_offset(ast->stsrc->type->members, ast->member);
            // the member existence is confirmed when analysis.
            assert(offset >= 0);

            int reg = x86_64_generate_code_detail(ast->stsrc);
            appcode(ADD(value(offset), nbyte_reg(8, reg)));
            return reg;
        }

        case AST_COMPOUND:
            for (int i = 0; i < vector_size(ast->stmts); i++)
                x86_64_generate_code_detail((AST *)vector_get(ast->stmts, i));
            return -1;

        case AST_EXPR_STMT:
            assert(temp_reg_table == 0);
            generate_basic_block_start_maker();
            if (ast->lhs != NULL) {
                int reg = x86_64_generate_code_detail(ast->lhs);
                if (reg != -1) restore_temp_reg(reg);
            }
            assert(temp_reg_table == 0);
            generate_basic_block_end_marker();
            return -1;

        case AST_ARY2PTR:
            return x86_64_generate_code_detail(ast->ary);

        case AST_CHAR2INT:
            return x86_64_generate_code_detail(ast->lhs);

        case AST_LVALUE2RVALUE: {
            int lreg = x86_64_generate_code_detail(ast->lhs),
                rreg = get_temp_reg();
            generate_mov_mem_reg(ast->type->nbytes, lreg, rreg);
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_LVAR_DECL_INIT:
        case AST_GVAR_DECL_INIT: {
            x86_64_generate_code_detail(ast->lhs);
            int rreg = x86_64_generate_code_detail(ast->rhs);
            if (rreg != -1) restore_temp_reg(rreg);
            return -1;
        }

        case AST_DECL_LIST:
            for (int i = 0; i < vector_size(ast->decls); i++)
                x86_64_generate_code_detail((AST *)vector_get(ast->decls, i));
            return -1;

        case AST_VA_START: {
            int reg = x86_64_generate_code_detail(ast->lhs);
            Code *reg_code = nbyte_reg(8, reg);
            // typedef struct {
            //    unsigned int gp_offset;
            //    unsigned int fp_offset;
            //    void *overflow_arg_area;
            //    void *reg_save_area;
            // } va_list[1];
            Code *gp_offset = addrof(reg_code, 0),
                 *fp_offset = addrof(reg_code, 4),
                 *overflow_arg_area = addrof(reg_code, 8),
                 *reg_save_area = addrof(reg_code, 16);

            assert(ast->rhs->kind == AST_INT);
            appcode(MOVL(value(ast->rhs->ival * 8), gp_offset));
            appcode(MOVL(value(48), fp_offset));
            appcode(LEA(addrof(RBP(), codeenv->overflow_arg_area_stack_idx),
                        RDI()));
            appcode(MOV(RDI(), overflow_arg_area));
            appcode(
                LEA(addrof(RBP(), codeenv->reg_save_area_stack_idx), RDI()));
            appcode(MOV(RDI(), reg_save_area));
            restore_temp_reg(reg);
            return -1;
        }

        case AST_VA_ARG_INT: {
            char *stack_label = make_label_string(),
                 *fetch_label = make_label_string();
            int reg = x86_64_generate_code_detail(ast->lhs);
            Code *reg_code = nbyte_reg(8, reg),
                 *gp_offset = addrof(reg_code, 0),
                 *overflow_arg_area = addrof(reg_code, 8),
                 *reg_save_area = addrof(reg_code, 16);
            appcode(MOV(gp_offset, EAX()));
            appcode(CMP(value(48), EAX()));
            appcode(JAE(stack_label));
            appcode(MOV(EAX(), EDX()));
            appcode(ADD(value(8), EDX()));
            appcode(ADD(reg_save_area, RAX()));
            appcode(MOV(EDX(), gp_offset));
            appcode(JMP(fetch_label));
            appcode(LABEL(stack_label));
            appcode(MOV(overflow_arg_area, RAX()));
            appcode(LEA(addrof(RAX(), 8), RDX()));
            appcode(MOV(RDX(), overflow_arg_area));
            appcode(LABEL(fetch_label));
            appcode(MOV(addrof(RAX(), 0), nbyte_reg(4, reg)));
            return reg;
        }

        case AST_VA_ARG_CHARP: {
            char *stack_label = make_label_string(),
                 *fetch_label = make_label_string();
            int reg = x86_64_generate_code_detail(ast->lhs);
            Code *reg_code = nbyte_reg(8, reg),
                 *gp_offset = addrof(reg_code, 0),
                 *overflow_arg_area = addrof(reg_code, 8),
                 *reg_save_area = addrof(reg_code, 16);
            appcode(MOV(gp_offset, EAX()));
            appcode(CMP(value(48), EAX()));
            appcode(JAE(stack_label));
            appcode(MOV(EAX(), EDX()));
            appcode(ADD(value(8), EDX()));
            appcode(ADD(reg_save_area, RAX()));
            appcode(MOV(EDX(), gp_offset));
            appcode(JMP(fetch_label));
            appcode(LABEL(stack_label));
            appcode(MOV(overflow_arg_area, RAX()));
            appcode(LEA(addrof(RAX(), 8), RDX()));
            appcode(MOV(RDX(), overflow_arg_area));
            appcode(LABEL(fetch_label));
            appcode(MOV(addrof(RAX(), 0), nbyte_reg(8, reg)));
            return reg;
        }

        case AST_GVAR_DECL:
        case AST_LVAR_DECL:
        case AST_FUNC_DECL:
        case AST_NOP:
            return -1;
    }

    warn("gen.c %d\n", ast->kind);
    assert(0);
}

Vector *x86_64_generate_code(Vector *asts)
{
    x86_64_analyze_ast(asts);

    init_code_env();

    appcode(new_code(CD_TEXT));

    for (int i = 0; i < vector_size(asts); i++)
        x86_64_generate_code_detail((AST *)vector_get(asts, i));

    appcode(new_code(CD_DATA));

    Vector *gvar_list = get_gvar_list();
    for (int i = 0; i < vector_size(gvar_list); i++) {
        GVar *gvar = (GVar *)vector_get(gvar_list, i);

        if (gvar->is_global) appcode(GLOBAL(gvar->name));
        appcode(LABEL(gvar->name));

        AST *value = gvar->value;

        if (value == NULL) {  // no initial value
            Code *code = new_code(CD_ZERO);
            code->ival = gvar->type->nbytes;
            appcode(code);
            continue;
        }

        switch (value->kind) {
            case AST_STRING_LITERAL:
                assert(gvar->type->kind == TY_ARY &&
                       gvar->type->ptr_of->kind == TY_CHAR);
                Code *code = new_code(CD_ASCII);
                code->sval = value->sval;
                code->ival = gvar->type->nbytes;
                appcode(code);
                break;

            case AST_CONSTANT: {
                assert(gvar->type->kind != TY_ARY);  // TODO: implement

                // TODO: I wonder if this can be done in
                // x86_64_analyze_ast().
                int ival = x86_64_eval_ast_int(gvar->value);

                int type2spec[16];  // TODO: enough length?
                type2spec[TY_INT] = CD_LONG;
                type2spec[TY_CHAR] = CD_BYTE;
                type2spec[TY_PTR] = CD_QUAD;

                Code *code = new_code(type2spec[gvar->type->kind]);
                code->ival = ival;
                appcode(code);
            } break;

            default:
                assert(0);
        }
    }

    return clone_vector(codeenv->code);
}

static AST *x86_64_optimize_ast_constant_detail(AST *ast, Env *env)
{
    if (ast == NULL) return ast;

    switch (ast->kind) {
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        case AST_REM:
        case AST_LSHIFT:
        case AST_RSHIFT:
        case AST_LT:
        case AST_LTE:
        case AST_EQ:
        case AST_AND:
        case AST_XOR:
        case AST_OR:
        case AST_LAND:
        case AST_LOR:
        case AST_NEQ: {
            ast->lhs = x86_64_optimize_ast_constant_detail(ast->lhs, env),
            ast->rhs = x86_64_optimize_ast_constant_detail(ast->rhs, env);
            if (ast->lhs->kind != AST_INT || ast->rhs->kind != AST_INT)
                return ast;
            // TODO: feature work: long
            int ret = 0;
            switch (ast->kind) {
                case AST_ADD:
                    ret = ast->lhs->ival + ast->rhs->ival;
                    break;
                case AST_SUB:
                    ret = ast->lhs->ival - ast->rhs->ival;
                    break;
                case AST_MUL:
                    ret = ast->lhs->ival * ast->rhs->ival;
                    break;
                case AST_DIV:
                    ret = ast->lhs->ival / ast->rhs->ival;
                    break;
                case AST_REM:
                    ret = ast->lhs->ival % ast->rhs->ival;
                    break;
                case AST_LSHIFT:
                    ret = ast->lhs->ival << ast->rhs->ival;
                    break;
                case AST_RSHIFT:
                    ret = ast->lhs->ival >> ast->rhs->ival;
                    break;
                case AST_LT:
                    ret = ast->lhs->ival < ast->rhs->ival;
                    break;
                case AST_LTE:
                    ret = ast->lhs->ival <= ast->rhs->ival;
                    break;
                case AST_EQ:
                    ret = ast->lhs->ival == ast->rhs->ival;
                    break;
                case AST_AND:
                    ret = ast->lhs->ival & ast->rhs->ival;
                    break;
                case AST_XOR:
                    ret = ast->lhs->ival ^ ast->rhs->ival;
                    break;
                case AST_OR:
                    ret = ast->lhs->ival | ast->rhs->ival;
                    break;
                case AST_LAND:
                    ret = ast->lhs->ival && ast->rhs->ival;
                    break;
                case AST_LOR:
                    ret = ast->lhs->ival || ast->rhs->ival;
                    break;
                case AST_NEQ:
                    ret = ast->lhs->ival != ast->rhs->ival;
                    break;
                default:
                    assert(0);
            }

            return new_int_ast(ret);
        }

        case AST_ASSIGN:
        case AST_LVAR_DECL_INIT:
        case AST_GVAR_DECL_INIT:
        case AST_ENUM_VAR_DECL_INIT:
            ast->lhs = x86_64_optimize_ast_constant_detail(ast->lhs, env);
            ast->rhs = x86_64_optimize_ast_constant_detail(ast->rhs, env);
            return ast;

        case AST_COMPL:
        case AST_UNARY_MINUS:
        case AST_NOT: {
            ast->lhs = x86_64_optimize_ast_constant_detail(ast->lhs, env);
            if (ast->lhs->kind != AST_INT) return ast;
            int ret = 0;
            switch (ast->kind) {
                case AST_COMPL:
                    ret = ~ast->lhs->ival;
                    break;
                case AST_UNARY_MINUS:
                    ret = -ast->lhs->ival;
                    break;
                case AST_NOT:
                    ret = !ast->lhs->ival;
                    break;
                default:
                    assert(0);
            }
            return new_int_ast(ret);
        }

        case AST_EXPR_STMT:
        case AST_RETURN:
        case AST_PREINC:
        case AST_POSTINC:
        case AST_PREDEC:
        case AST_POSTDEC:
        case AST_ADDR:
        case AST_INDIR:
        case AST_CAST:
        case AST_LVALUE2RVALUE:
            ast->lhs = x86_64_optimize_ast_constant_detail(ast->lhs, env);
            return ast;

        case AST_ARY2PTR:
            ast->ary = x86_64_optimize_ast_constant_detail(ast->ary, env);
            return ast;

        case AST_COND:
            ast->cond = x86_64_optimize_ast_constant_detail(ast->cond, env);
            ast->then = x86_64_optimize_ast_constant_detail(ast->then, env);
            ast->els = x86_64_optimize_ast_constant_detail(ast->els, env);
            return ast;

        case AST_EXPR_LIST:
            for (int i = 0; i < vector_size(ast->exprs); i++)
                vector_set(ast->exprs, i,
                           x86_64_optimize_ast_constant_detail(
                               (AST *)vector_get(ast->exprs, i), env));
            return ast;

        case AST_VAR:
            return ast;

        case AST_DECL_LIST:
            for (int i = 0; i < vector_size(ast->decls); i++)
                vector_set(ast->decls, i,
                           x86_64_optimize_ast_constant_detail(
                               (AST *)vector_get(ast->decls, i), env));
            return ast;

        case AST_FUNCCALL:
            for (int i = 0; i < vector_size(ast->args); i++)
                vector_set(ast->args, i,
                           x86_64_optimize_ast_constant_detail(
                               (AST *)vector_get(ast->args, i), env));
            return ast;

        case AST_FUNCDEF:
            ast->body = x86_64_optimize_ast_constant_detail(ast->body, env);
            return ast;

        case AST_COMPOUND:
            for (int i = 0; i < vector_size(ast->stmts); i++)
                vector_set(ast->stmts, i,
                           x86_64_optimize_ast_constant_detail(
                               (AST *)vector_get(ast->stmts, i), env));
            return ast;

        case AST_IF:
            ast->cond = x86_64_optimize_ast_constant_detail(ast->cond, env);
            ast->then = x86_64_optimize_ast_constant_detail(ast->then, env);
            ast->els = x86_64_optimize_ast_constant_detail(ast->els, env);
            return ast;

        case AST_LABEL:
            ast->label_stmt =
                x86_64_optimize_ast_constant_detail(ast->label_stmt, env);
            return ast;

        case AST_CASE:
            ast->lhs = x86_64_optimize_ast_constant_detail(ast->lhs, env);
            ast->rhs = x86_64_optimize_ast_constant_detail(ast->rhs, env);
            return ast;

        case AST_DEFAULT:
            ast->lhs = x86_64_optimize_ast_constant_detail(ast->lhs, env);
            return ast;

        case AST_SWITCH:
            ast->target = x86_64_optimize_ast_constant_detail(ast->target, env);
            ast->switch_body =
                x86_64_optimize_ast_constant_detail(ast->switch_body, env);
            return ast;

        case AST_DOWHILE:
            ast->cond = x86_64_optimize_ast_constant_detail(ast->cond, env);
            ast->then = x86_64_optimize_ast_constant_detail(ast->then, env);
            return ast;

        case AST_FOR:
            ast->initer = x86_64_optimize_ast_constant_detail(ast->initer, env);
            ast->midcond =
                x86_64_optimize_ast_constant_detail(ast->midcond, env);
            ast->iterer = x86_64_optimize_ast_constant_detail(ast->iterer, env);
            ast->for_body =
                x86_64_optimize_ast_constant_detail(ast->for_body, env);
            return ast;

        case AST_MEMBER_REF:
            ast->stsrc = x86_64_optimize_ast_constant_detail(ast->stsrc, env);
            return ast;
    }

    return ast;
}

static AST *x86_64_optimize_ast_constant(AST *ast, Env *env)
{
    return x86_64_optimize_ast_constant_detail(ast, env);
}

void x86_64_optimize_asts_constant(Vector *asts, Env *env)
{
    for (int i = 0; i < vector_size(asts); i++)
        vector_set(
            asts, i,
            x86_64_optimize_ast_constant((AST *)vector_get(asts, i), env));
}

static int get_using_register(Code *code)
{
    if (code == NULL) return -1;
    switch (code->kind) {
        case REG_AL:
        case REG_DIL:
        case REG_SIL:
        case REG_DL:
        case REG_CL:
        case REG_R8B:
        case REG_R9B:
        case REG_R10B:
        case REG_R11B:
        case REG_R12B:
        case REG_R13B:
        case REG_R14B:
        case REG_R15B:
        case REG_BPL:
        case REG_SPL:
        case REG_AX:
        case REG_DI:
        case REG_SI:
        case REG_DX:
        case REG_CX:
        case REG_R8W:
        case REG_R9W:
        case REG_R10W:
        case REG_R11W:
        case REG_R12W:
        case REG_R13W:
        case REG_R14W:
        case REG_R15W:
        case REG_BP:
        case REG_SP:
        case REG_EAX:
        case REG_EDI:
        case REG_ESI:
        case REG_EDX:
        case REG_ECX:
        case REG_R8D:
        case REG_R9D:
        case REG_R10D:
        case REG_R11D:
        case REG_R12D:
        case REG_R13D:
        case REG_R14D:
        case REG_R15D:
        case REG_EBP:
        case REG_ESP:
        case REG_RAX:
        case REG_RDI:
        case REG_RSI:
        case REG_RDX:
        case REG_RCX:
        case REG_R8:
        case REG_R9:
        case REG_R10:
        case REG_R11:
        case REG_R12:
        case REG_R13:
        case REG_R14:
        case REG_R15:
        case REG_RBP:
        case REG_RSP:
        case REG_RIP:
            return code->kind;
        case CD_ADDR_OF:
            return get_using_register(code->lhs);
        case CD_ADDR_OF_LABEL:
            return get_using_register(code->lhs);
    }

    return -1;
}

static int is_addrof_code(Code *code)
{
    if (code == NULL) return 0;
    return code->kind == CD_ADDR_OF || code->kind == CD_ADDR_OF_LABEL;
}

static int is_same_code(Code *lhs, Code *rhs)
{
    if (lhs == NULL || rhs == NULL) return lhs == rhs;
    if (lhs->kind != rhs->kind) return 0;
    if (!is_same_code(lhs->lhs, rhs->lhs) || !is_same_code(lhs->rhs, rhs->rhs))
        return 0;
    if (lhs->ival != rhs->ival) return 0;
    if (lhs->label != rhs->label) return 0;  // TODO: strcmp
    return 1;
}

static Vector *x86_64_optimize_code_detail_propagation(Vector *block)
{
    Vector *nblock = new_vector();
    for (int i = 0; i < vector_size(block); i++) {
        Code *code = (Code *)vector_get(block, i);

        switch (code->kind) {
            case INST_LEA:
                // lea mem, reg
                // mov val, (reg)
                if (is_addrof_code(code->lhs) && is_register_code(code->rhs) &&
                    i != vector_size(block) - 1) {
                    Code *next_code = (Code *)vector_get(block, i + 1);
                    if (next_code->kind == INST_MOV &&
                        is_addrof_code(next_code->lhs) &&
                        is_register_code(next_code->rhs)) {
                        if (is_same_code(code->rhs, next_code->lhs->lhs)) {
                            next_code->lhs = code->lhs;
                            vector_set(next_code->read_dep, 0, code->lhs);
                        }
                    }
                }
                vector_push_back(nblock, code);
                break;

            default:
                vector_push_back(nblock, code);
                break;
        }
    }
    return nblock;
}

static Vector *x86_64_optimize_code_detail_eliminate(Vector *block)
{
    Vector *nblock = new_vector();
    int used_reg_flag = 0;
    for (int i = vector_size(block) - 1; i >= 0; i--) {
        Code *code = (Code *)vector_get(block, i);

        switch (code->kind) {
            case INST_LEA:
            case INST_MOV: {
                if (!code->can_be_eliminated || !is_register_code(code->rhs) ||
                    used_reg_flag & (1 << (code->rhs->kind & 31)))
                    vector_push_back(nblock, code);
                if (is_register_code(code->rhs))
                    used_reg_flag &= ~(1 << (code->rhs->kind & 31));
            } break;

            default:
                vector_push_back(nblock, code);
                break;
        }

        for (int i = 0; i < vector_size(code->read_dep); i++) {
            int reg = get_using_register(vector_get(code->read_dep, i));
            if (reg != -1) used_reg_flag |= 1 << (reg & 31);
        }
    }

    // reverse nblock
    int size = vector_size(nblock);
    for (int i = 0; i < size / 2; i++) {
        Code *lhs = (Code *)vector_get(nblock, i),
             *rhs = (Code *)vector_get(nblock, size - i - 1);
        vector_set(nblock, i, rhs);
        vector_set(nblock, size - i - 1, lhs);
    }

    return nblock;
}

static int are_different_vectors(Vector *lhs, Vector *rhs)
{
    if (lhs == NULL || rhs == NULL) return lhs != rhs;
    int size = vector_size(lhs);
    if (size != vector_size(rhs)) return 1;
    for (int i = 0; i < size; i++)
        if (vector_get(lhs, i) != vector_get(rhs, i)) return 1;
    return 0;
}

static Vector *x86_64_optimize_code_detail_basic_block(Vector *block)
{
    Vector *org_block = block, *nblock = block;
    do {
        org_block = nblock;
        nblock = x86_64_optimize_code_detail_propagation(nblock);
        nblock = x86_64_optimize_code_detail_eliminate(nblock);
    } while (are_different_vectors(org_block, nblock));  // do-while

    return nblock;
}

static int x86_64_optimize_code_detail_funcdef(Vector *scode, int index,
                                               Vector *ncode)
{
    int used = -1, end_index = -1;
    for (int i = index; i < vector_size(scode); i++) {
        Code *code = vector_get(scode, i);
        if (code->kind == MRK_FUNCDEF_START) continue;
        if (code->kind == MRK_FUNCDEF_END) {
            end_index = i;
            break;
        }
        if (code->kind == MRK_FUNCDEF_RETURN) continue;

        // TODO: magic number
        int reg = get_using_register(code->lhs);
        if (reg != -1) {
            if (reg_of_nbyte(8, reg) == REG_R12) used = max(used, 0);
            if (reg_of_nbyte(8, reg) == REG_R13) used = max(used, 1);
            if (reg_of_nbyte(8, reg) == REG_R14) used = max(used, 2);
            if (reg_of_nbyte(8, reg) == REG_R15) used = max(used, 3);
        }
        reg = get_using_register(code->rhs);
        if (reg != -1) {
            if (reg_of_nbyte(8, reg) == REG_R12) used = max(used, 0);
            if (reg_of_nbyte(8, reg) == REG_R13) used = max(used, 1);
            if (reg_of_nbyte(8, reg) == REG_R14) used = max(used, 2);
            if (reg_of_nbyte(8, reg) == REG_R15) used = max(used, 3);
        }
    }
    assert(end_index != -1);

    // output
    if (used >= 3) vector_push_back(ncode, PUSH(R15()));
    if (used >= 2) vector_push_back(ncode, PUSH(R14()));
    if (used >= 1) vector_push_back(ncode, PUSH(R13()));
    if (used >= 0) vector_push_back(ncode, PUSH(R12()));
    for (int i = index + 1; i < end_index; i++) {
        Code *code = vector_get(scode, i);
        switch (code->kind) {
            case MRK_FUNCDEF_RETURN:
                if (used >= 0) vector_push_back(ncode, POP(R12()));
                if (used >= 1) vector_push_back(ncode, POP(R13()));
                if (used >= 2) vector_push_back(ncode, POP(R14()));
                if (used >= 3) vector_push_back(ncode, POP(R15()));
                break;

            default:
                vector_push_back(ncode, code);
        }
    }
    if (used >= 0) vector_push_back(ncode, POP(R12()));
    if (used >= 1) vector_push_back(ncode, POP(R13()));
    if (used >= 2) vector_push_back(ncode, POP(R14()));
    if (used >= 3) vector_push_back(ncode, POP(R15()));

    return end_index;
}

Vector *x86_64_optimize_code(Vector *code)
{
    Vector *ncode = new_vector();

    for (int i = 0; i < vector_size(code); i++) {
        Code *code0 = vector_get(code, i);
        switch (code0->kind) {
            case MRK_FUNCDEF_START:
                i = x86_64_optimize_code_detail_funcdef(code, i, ncode);
                break;
            default:
                vector_push_back(ncode, code0);
                break;
        }
    }

    code = ncode;
    ncode = new_vector();
    for (int i = 0; i < vector_size(code); i++) {
        Code *code0 = vector_get(code, i);
        switch (code0->kind) {
            case MRK_BASIC_BLOCK_START: {
                // create basic block
                Vector *block = new_vector();
                for (i++; i < vector_size(code); i++) {
                    Code *code2 = vector_get(code, i);
                    if (code2->kind == MRK_BASIC_BLOCK_END) break;
                    vector_push_back(block, code2);
                }

                // optimize the block
                vector_push_back_vector(
                    ncode, x86_64_optimize_code_detail_basic_block(block));
            } break;

            case MRK_FUNCDEF_START:
                i = x86_64_optimize_code_detail_funcdef(code, i, ncode);
                break;

            default:
                vector_push_back(ncode, code0);
                break;
        }
    }
    return ncode;
}
