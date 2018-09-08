#include "aqcc.h"

Code *new_code(int kind)
{
    Code *code = safe_malloc(sizeof(Code));
    code->kind = kind;
    code->lhs = code->rhs = NULL;
    code->ival = 0;
    code->sval = NULL;
    code->label = NULL;
    code->other_op = NULL;
    code->read_dep = new_vector();
    code->can_be_eliminated = 1;
    return code;
}

Code *new_binop_code(int kind, Code *lhs, Code *rhs)
{
    Code *code = new_code(kind);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    vector_push_back(code->read_dep, rhs);
    return code;
}

Code *new_unary_code(int kind, Code *lhs)
{
    Code *code = new_code(kind);
    code->lhs = lhs;
    code->rhs = NULL;
    vector_push_back(code->read_dep, lhs);
    return code;
}

Code *new_other_code(char *other_op, Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_OTHER);
    code->other_op = other_op;
    code->lhs = lhs;
    code->rhs = rhs;
    if (lhs) vector_push_back(code->read_dep, lhs);
    if (rhs) vector_push_back(code->read_dep, rhs);
    return code;
}

Code *new_value_code(int value)
{
    Code *code = new_code(CD_VALUE);
    code->ival = value;
    return code;
}

Code *new_addrof_label_code(Code *reg, char *label)
{
    Code *code = new_code(CD_ADDR_OF_LABEL);
    code->lhs = reg;
    code->label = label;
    return code;
}

Code *new_addrof_code(Code *reg, int offset)
{
    Code *code = new_code(CD_ADDR_OF);
    code->lhs = reg;
    code->ival = offset;
    return code;
}

Code *MOV(Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_MOV);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    if (!is_register_code(rhs)) vector_push_back(code->read_dep, rhs);
    return code;
}

Code *MOVL(Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_MOVL);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    if (!is_register_code(rhs)) vector_push_back(code->read_dep, rhs);
    return code;
}

Code *MOVSBL(Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_MOVSBL);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    if (!is_register_code(rhs)) vector_push_back(code->read_dep, rhs);
    return code;
}

Code *MOVSLQ(Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_MOVSLQ);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    if (!is_register_code(rhs)) vector_push_back(code->read_dep, rhs);
    return code;
}

Code *MOVZB(Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_MOVZB);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    if (!is_register_code(rhs)) vector_push_back(code->read_dep, rhs);
    return code;
}

Code *LEA(Code *lhs, Code *rhs)
{
    Code *code = new_code(INST_LEA);
    code->lhs = lhs;
    code->rhs = rhs;
    vector_push_back(code->read_dep, lhs);
    if (!is_register_code(rhs)) vector_push_back(code->read_dep, rhs);
    return code;
}

Code *PUSH(Code *lhs) { return new_unary_code(INST_PUSH, lhs); }

Code *POP(Code *lhs) { return new_unary_code(INST_POP, lhs); }

Code *ADD(Code *lhs, Code *rhs) { return new_binop_code(INST_ADD, lhs, rhs); }

Code *ADDQ(Code *lhs, Code *rhs) { return new_binop_code(INST_ADDQ, lhs, rhs); }

Code *SUB(Code *lhs, Code *rhs) { return new_binop_code(INST_SUB, lhs, rhs); }

Code *IMUL(Code *lhs, Code *rhs) { return new_binop_code(INST_IMUL, lhs, rhs); }

Code *IDIV(Code *lhs) { return new_unary_code(INST_IDIV, lhs); }

Code *SAR(Code *lhs, Code *rhs) { return new_binop_code(INST_SAR, lhs, rhs); }

Code *SAL(Code *lhs, Code *rhs) { return new_binop_code(INST_SAL, lhs, rhs); }

Code *NEG(Code *lhs) { return new_unary_code(INST_NEG, lhs); }

Code *NOT(Code *lhs) { return new_unary_code(INST_NOT, lhs); }

Code *CMP(Code *lhs, Code *rhs) { return new_binop_code(INST_CMP, lhs, rhs); }

Code *SETL(Code *lhs) { return new_unary_code(INST_SETL, lhs); }

Code *SETLE(Code *lhs) { return new_unary_code(INST_SETLE, lhs); }

Code *SETE(Code *lhs) { return new_unary_code(INST_SETE, lhs); }

Code *AND(Code *lhs, Code *rhs) { return new_binop_code(INST_AND, lhs, rhs); }

Code *XOR(Code *lhs, Code *rhs) { return new_binop_code(INST_XOR, lhs, rhs); }

Code *OR(Code *lhs, Code *rhs) { return new_binop_code(INST_OR, lhs, rhs); }

Code *RET() { return new_code(INST_RET); }

Code *CLTD() { return new_code(INST_CLTD); }

Code *CLTQ() { return new_code(INST_CLTQ); }

Code *JMP(char *label)
{
    Code *code = new_code(INST_JMP);
    code->label = label;
    return code;
}

Code *JE(char *label)
{
    Code *code = new_code(INST_JE);
    code->label = label;
    return code;
}

Code *JNE(char *label)
{
    Code *code = new_code(INST_JNE);
    code->label = label;
    return code;
}

Code *JAE(char *label)
{
    Code *code = new_code(INST_JAE);
    code->label = label;
    return code;
}

Code *LABEL(char *label)
{
    Code *code = new_code(INST_LABEL);
    code->label = label;
    return code;
}

Code *INCL(Code *lhs) { return new_unary_code(INST_INCL, lhs); }

Code *INCQ(Code *lhs) { return new_unary_code(INST_INCQ, lhs); }

Code *DECL(Code *lhs) { return new_unary_code(INST_DECL, lhs); }

Code *DECQ(Code *lhs) { return new_unary_code(INST_DECQ, lhs); }

Code *EAX() { return new_code(REG_EAX); }

Code *EDX() { return new_code(REG_EDX); }

Code *RAX() { return new_code(REG_RAX); }

Code *RBP() { return new_code(REG_RBP); }

Code *RSP() { return new_code(REG_RSP); }

Code *RIP() { return new_code(REG_RIP); }

Code *RDI() { return new_code(REG_RDI); }

Code *RDX() { return new_code(REG_RDX); }

Code *R10() { return new_code(REG_R10); }

Code *R11() { return new_code(REG_R11); }

Code *R12() { return new_code(REG_R12); }

Code *R13() { return new_code(REG_R13); }

Code *R14() { return new_code(REG_R14); }

Code *R15() { return new_code(REG_R15); }

Code *AL() { return new_code(REG_AL); }

Code *CL() { return new_code(REG_CL); }

Code *GLOBAL(char *label)
{
    Code *code = new_code(CD_GLOBAL);
    code->label = label;
    return code;
}

char *code2str(Code *code)
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

        case INST_OTHER: {
            char *lhs = code2str(code->lhs), *rhs = code2str(code->rhs);
            char *ret = code->other_op;
            if (lhs) ret = format("%s %s", ret, lhs);
            if (rhs) ret = format("%s, %s", ret, rhs);
            return ret;
        }

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
