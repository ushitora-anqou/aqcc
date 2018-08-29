#include "aqcc.h"

Code *new_code(int kind)
{
    Code *code = safe_malloc(sizeof(Code));
    code->kind = kind;
    code->lhs = code->rhs = NULL;
    code->ival = 0;
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

Code *value(int value)
{
    Code *code = new_code(CD_VALUE);
    code->ival = value;
    return code;
}

Code *addrof_label(Code *reg, char *label)
{
    Code *code = new_code(CD_ADDR_OF_LABEL);
    code->lhs = reg;
    code->label = label;
    return code;
}

Code *addrof(Code *reg, int offset)
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

Code *EAX() { return new_code(REG_EAX); }

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

int temp_reg_table;

void init_temp_reg() { temp_reg_table = 0; }

int get_temp_reg()
{
    for (int i = 0; i < 6; i++) {
        if (temp_reg_table & (1 << i)) continue;
        temp_reg_table |= (1 << i);
        return i + 7;  // corresponding to reg_name's index
    }

    error("no more register");
}

int restore_temp_reg(int i) { temp_reg_table &= ~(1 << (i - 7)); }

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

        case INST_OTHER: {
            char *lhs = code2str(code->lhs), *rhs = code2str(code->rhs);
            char *ret = code->other_op;
            if (lhs) ret = format("%s %s", ret, lhs);
            if (rhs) ret = format("%s, %s", ret, rhs);
            return ret;
        }

        case CD_VALUE:
            return format("$%d", code->ival);

        case CD_ADDR_OF:
            if (code->ival == 0) return format("(%s)", code2str(code->lhs));
            return format("%d(%s)", code->ival, code2str(code->lhs));

        case CD_ADDR_OF_LABEL:
            return format("%s(%s)", code->label, code2str(code->lhs));
    }
    warn(format("%d", code->kind));
    assert(0);
}

const char *reg_name(int byte, int i)
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

char byte2suffix(int byte)
{
    switch (byte) {
        case 8:
            return 'q';
        case 4:
            return 'l';
        default:
            assert(0);
    }
}

int lookup_member_offset(Vector *members, char *member)
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

void generate_code_detail(AST *ast);

void init_code_env()
{
    codeenv = (CodeEnv *)safe_malloc(sizeof(CodeEnv));
    codeenv->continue_label = codeenv->break_label = NULL;
    codeenv->reg_save_area_stack_idx = 0;
    codeenv->code = new_vector();
}

void appcode(Code *code) { vector_push_back(codeenv->code, code); }

void appcode_str(const char *src, ...)
{
    char buf[256], buf2[256];  // TODO: enough length?
    int i, bufidx;
    va_list args;

    // copy src to buf.
    // replace # to %% in src.
    for (i = 0, bufidx = 0;; i++) {
        if (src[i] == '\0') break;

        if (src[i] == '#') {
            buf[bufidx++] = '%';
            buf[bufidx++] = '%';
            continue;
        }

        buf[bufidx++] = src[i];
    }
    buf[bufidx] = '\0';

    va_start(args, src);
    vsprintf(buf2, buf, args);
    va_end(args);

    appcode(new_other_code(new_str(buf2), NULL, NULL));
}

void dump_code(Code *code, FILE *fh)
{  //
    fprintf(fh, "%s\n", code2str(code));
}

Code *last_appended_code()
{
    for (int i = vector_size(codeenv->code) - 1; i >= 0; i--) {
        Code *str = vector_get(codeenv->code, i);
        if (str) return str;
    }

    assert(0);
}

void add_read_dep(Code *dep)
{
    vector_push_back(last_appended_code()->read_dep, dep);
}

void disable_elimination() { last_appended_code()->can_be_eliminated = 0; }

int nbyte_of_reg(int reg)
{
    int ret = (reg & (REG_8 | REG_16 | REG_32 | REG_64)) >> 5;
    assert(ret == 1 || ret == 2 || ret == 3 || ret == 4);
    return ret;
}

void generate_basic_block_start_maker()
{
    appcode_str("/* BLOCK START */");
    appcode(new_code(MRK_BASIC_BLOCK_START));
}

void generate_basic_block_end_marker()
{
    appcode(new_code(MRK_BASIC_BLOCK_END));
    appcode_str("/* BLOCK END */");
}

void generate_funcdef_start_marker()
{
    appcode_str("/* FUNCDEF START */");
    appcode(new_code(MRK_FUNCDEF_START));
}

void generate_funcdef_end_marker()
{
    appcode(new_code(MRK_FUNCDEF_END));
    appcode_str("/* FUNCDEF END */");
}

void generate_funcdef_return_marker() { appcode(new_code(MRK_FUNCDEF_RETURN)); }

Code *nbyte_reg(int nbyte, int reg)
{
    return new_code(reg_of_nbyte(nbyte, reg));
}

void generate_mov_mem_reg(int nbyte, int src_reg, int dst_reg)
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

int generate_register_code_detail(AST *ast)
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
                int reg = generate_register_code_detail(ast->lhs);
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
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);

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
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);

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
                appcode(SAR(value(2), nbyte_reg(8, lreg)));
            }
            else {
                int nbytes = ast->type->nbytes;
                appcode(SUB(nbyte_reg(nbytes, rreg), nbyte_reg(nbytes, lreg)));
            }

            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_MUL: {
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);
            int nbytes = 4;  // TODO: long
            appcode(IMUL(nbyte_reg(nbytes, lreg), nbyte_reg(nbytes, rreg)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_DIV: {
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);
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
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);
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
            int reg = generate_register_code_detail(ast->lhs);
            appcode(NEG(nbyte_reg(ast->type->nbytes, reg)));
            return reg;
        }

        case AST_COMPL: {
            int reg = generate_register_code_detail(ast->lhs);
            appcode(NOT(nbyte_reg(ast->type->nbytes, reg)));
            return reg;
        }

        case AST_LSHIFT: {
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);
            appcode(MOV(nbyte_reg(1, rreg), CL()));
            appcode(SAL(CL(), nbyte_reg(ast->type->nbytes, lreg)));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_RSHIFT: {
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);
            appcode(MOV(nbyte_reg(1, rreg), CL()));
            appcode(SAR(CL(), nbyte_reg(ast->type->nbytes, lreg)));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_LT: {
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);
            appcode(CMP(nbyte_reg(ast->type->nbytes, rreg),
                        nbyte_reg(ast->type->nbytes, lreg)));
            appcode(SETL(AL()));
            appcode(MOVZB(AL(), nbyte_reg(ast->type->nbytes, lreg)));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_LTE: {
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);
            appcode(CMP(nbyte_reg(ast->type->nbytes, rreg),
                        nbyte_reg(ast->type->nbytes, lreg)));
            appcode(SETLE(AL()));
            appcode(MOVZB(AL(), nbyte_reg(ast->type->nbytes, lreg)));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_EQ: {
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);
            appcode(CMP(nbyte_reg(ast->type->nbytes, rreg),
                        nbyte_reg(ast->type->nbytes, lreg)));
            appcode(SETE(AL()));
            appcode(MOVZB(AL(), nbyte_reg(ast->type->nbytes, lreg)));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_AND: {
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);
            appcode(AND(nbyte_reg(ast->type->nbytes, lreg),
                        nbyte_reg(ast->type->nbytes, rreg)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_XOR: {
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);
            appcode(XOR(nbyte_reg(ast->type->nbytes, lreg),
                        nbyte_reg(ast->type->nbytes, rreg)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_OR: {
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);
            appcode(OR(nbyte_reg(ast->type->nbytes, lreg),
                       nbyte_reg(ast->type->nbytes, rreg)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_LAND: {
            char *false_label = make_label_string(),
                 *exit_label = make_label_string();
            int lreg = generate_register_code_detail(ast->lhs);
            restore_temp_reg(lreg);
            appcode(CMP(value(0), nbyte_reg(ast->type->nbytes, lreg)));
            appcode(JE(false_label));
            // don't execute rhs expression if lhs is false.
            int rreg = generate_register_code_detail(ast->rhs);
            Code *rreg_code = nbyte_reg(ast->type->nbytes, rreg);
            appcode(CMP(value(0), rreg_code));
            appcode(JE(false_label));
            appcode(MOV(value(1), rreg_code));
            appcode(JMP(exit_label));
            appcode_str("%s:", false_label);
            appcode(MOV(value(0), rreg_code));
            appcode_str("%s:", exit_label);
            return rreg;
        }

        case AST_LOR: {
            char *true_label = make_label_string(),
                 *exit_label = make_label_string();
            int lreg = generate_register_code_detail(ast->lhs);
            restore_temp_reg(lreg);
            appcode(CMP(value(0), nbyte_reg(ast->type->nbytes, lreg)));
            appcode(JNE(true_label));
            // don't execute rhs expression if lhs is true.
            int rreg = generate_register_code_detail(ast->rhs);
            Code *rreg_code = nbyte_reg(ast->type->nbytes, rreg);
            appcode(CMP(value(0), rreg_code));
            appcode(JNE(true_label));
            appcode(CMP(value(0), rreg_code));
            appcode(JMP(exit_label));
            appcode_str("%s:", true_label);
            appcode(MOV(value(1), rreg_code));
            appcode_str("%s:", exit_label);
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
            appcode_str(".global %s", ast->fname);
            appcode_str("%s:", ast->fname);
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
            generate_register_code_detail(ast->body);
            assert(temp_reg_table == 0);
            RESTORE_VARIADIC_CXT;

            // avoid duplicate needless `ret`
            Code *code = last_appended_code();
            if (code->kind == INST_OTHER && strcmp(code->other_op, "ret") == 0)
                return -1;

            if (ast->type->kind != TY_VOID) appcode(MOV(value(0), RAX()));
            generate_funcdef_end_marker();
            appcode(MOV(RBP(), RSP()));
            appcode(POP(RBP()));
            appcode(RET());
            return -1;
        }

        case AST_ASSIGN: {
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = generate_register_code_detail(ast->rhs);

            appcode(MOV(nbyte_reg(ast->type->nbytes, rreg),
                        addrof(nbyte_reg(8, lreg), 0)));
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_EXPR_LIST: {
            int reg;
            for (int i = 0; i < vector_size(ast->exprs); i++) {
                reg = generate_register_code_detail(
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
                int reg = generate_register_code_detail(
                    (AST *)(vector_get(ast->args, i)));
                appcode(PUSH(nbyte_reg(8, reg)));
                restore_temp_reg(reg);
            }
            for (int i = 0; i < min(6, vector_size(ast->args)); i++)
                appcode(POP(nbyte_reg(8, i + 1)));
            appcode(MOV(value(0), EAX()));
            disable_elimination();
            appcode_str("call %s@PLT", ast->fname);
            appcode(ADD(value(8 * max(0, vector_size(ast->args) - 6)), RSP()));
            appcode(POP(R11()));
            appcode(POP(R10()));
            int reg = get_temp_reg();
            appcode(MOV(RAX(), nbyte_reg(8, reg)));
            return reg;
        }

        case AST_POSTINC: {
            char suf = byte2suffix(ast->lhs->type->nbytes);

            int lreg = generate_register_code_detail(ast->lhs);
            Code *lreg_code = nbyte_reg(8, lreg);
            int reg = get_temp_reg();
            generate_mov_mem_reg(ast->type->nbytes, lreg, reg);

            if (match_type(ast->lhs, TY_PTR))
                appcode(new_other_code(format("add%c", suf),
                                       value(ast->lhs->type->ptr_of->nbytes),
                                       addrof(lreg_code, 0)));
            else
                appcode(new_other_code(format("inc%c", suf),
                                       addrof(lreg_code, 0), NULL));

            restore_temp_reg(lreg);
            return reg;
        }

        case AST_PREINC: {
            char suf = byte2suffix(ast->lhs->type->nbytes);

            int lreg = generate_register_code_detail(ast->lhs);
            Code *lreg_code = nbyte_reg(8, lreg);
            int reg = get_temp_reg();

            if (match_type(ast->lhs, TY_PTR))
                appcode(new_other_code(format("add%c", suf),
                                       value(ast->lhs->type->ptr_of->nbytes),
                                       addrof(lreg_code, 0)));
            else
                appcode(new_other_code(format("inc%c", suf),
                                       addrof(lreg_code, 0), NULL));

            generate_mov_mem_reg(ast->type->nbytes, lreg, reg);

            restore_temp_reg(lreg);
            return reg;
        }

        case AST_POSTDEC: {
            char suf = byte2suffix(ast->lhs->type->nbytes);

            int lreg = generate_register_code_detail(ast->lhs);
            Code *lreg_code = nbyte_reg(8, lreg);
            int reg = get_temp_reg();
            generate_mov_mem_reg(ast->type->nbytes, lreg, reg);

            if (match_type(ast->lhs, TY_PTR))
                appcode(new_other_code(format("sub%c", suf),
                                       value(ast->lhs->type->ptr_of->nbytes),
                                       addrof(lreg_code, 0)));
            else
                appcode(new_other_code(format("dec%c", suf),
                                       addrof(lreg_code, 0), NULL));

            restore_temp_reg(lreg);
            return reg;
        }

        case AST_PREDEC: {
            char suf = byte2suffix(ast->lhs->type->nbytes);

            int lreg = generate_register_code_detail(ast->lhs);
            Code *lreg_code = nbyte_reg(8, lreg);
            int reg = get_temp_reg();

            if (match_type(ast->lhs, TY_PTR))
                appcode(new_other_code(format("sub%c", suf),
                                       value(ast->lhs->type->ptr_of->nbytes),
                                       addrof(lreg_code, 0)));
            else
                appcode(new_other_code(format("dec%c", suf),
                                       addrof(lreg_code, 0), NULL));

            generate_mov_mem_reg(ast->type->nbytes, lreg, reg);

            restore_temp_reg(lreg);
            return reg;
        }

        case AST_ADDR:
        case AST_INDIR:
            return generate_register_code_detail(ast->lhs);

        case AST_COND: {
            char *false_label = make_label_string(),
                 *exit_label = make_label_string();

            int cond_reg = generate_register_code_detail(ast->cond);
            restore_temp_reg(cond_reg);
            appcode(MOV(nbyte_reg(8, cond_reg), RAX()));
            appcode(CMP(value(0), EAX()));
            appcode(JE(false_label));
            int then_reg = generate_register_code_detail(ast->then);
            restore_temp_reg(then_reg);
            appcode(PUSH(nbyte_reg(8, then_reg)));
            appcode(JMP(exit_label));
            appcode_str("%s:", false_label);
            if (ast->els != NULL) {
                int els_reg = generate_register_code_detail(ast->els);
                restore_temp_reg(els_reg);
                appcode(PUSH(nbyte_reg(8, els_reg)));
            }
            appcode_str("%s:", exit_label);
            int reg = get_temp_reg();
            appcode(POP(nbyte_reg(8, reg)));
            return reg;
        }

        case AST_IF: {
            char *false_label = make_label_string(),
                 *exit_label = make_label_string();

            int cond_reg = generate_register_code_detail(ast->cond);
            restore_temp_reg(cond_reg);
            appcode(MOV(nbyte_reg(8, cond_reg), RAX()));
            appcode(CMP(value(0), EAX()));
            appcode(JE(false_label));
            generate_register_code_detail(ast->then);
            appcode(JMP(exit_label));
            appcode_str("%s:", false_label);
            if (ast->els != NULL) generate_register_code_detail(ast->els);
            appcode_str("%s:", exit_label);
            return -1;
        }

        case AST_SWITCH: {
            int target_reg = generate_register_code_detail(ast->target);
            restore_temp_reg(target_reg);
            char *name = reg_name(ast->target->type->nbytes, target_reg);

            for (int i = 0; i < vector_size(ast->cases); i++) {
                SwitchCase *cas = (SwitchCase *)vector_get(ast->cases, i);
                appcode(CMP(value(cas->cond),
                            nbyte_reg(ast->target->type->nbytes, target_reg)));
                // case has been already labeled when analyzing.
                appcode(JE(cas->label_name));
            }
            char *exit_label = make_label_string();
            if (ast->default_label)
                appcode(JMP(ast->default_label));
            else
                appcode(JMP(exit_label));

            SAVE_BREAK_CXT;
            codeenv->break_label = exit_label;
            generate_register_code_detail(ast->switch_body);
            appcode_str("%s:", exit_label);
            RESTORE_BREAK_CXT;

            return -1;
        }

        case AST_DOWHILE: {
            SAVE_BREAK_CXT;
            SAVE_CONTINUE_CXT;
            codeenv->break_label = make_label_string();
            codeenv->continue_label = make_label_string();
            char *start_label = make_label_string();

            appcode_str("%s:", start_label);
            generate_register_code_detail(ast->then);
            appcode_str("%s:", codeenv->continue_label);
            int cond_reg = generate_register_code_detail(ast->cond);
            appcode(
                CMP(value(0), nbyte_reg(ast->cond->type->nbytes, cond_reg)));
            appcode(JNE(start_label));
            appcode_str("%s:", codeenv->break_label);

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
                int reg = generate_register_code_detail(ast->initer);
                if (reg != -1) restore_temp_reg(reg);  // if expr
            }
            appcode_str("%s:", start_label);
            if (ast->midcond != NULL) {
                int reg = generate_register_code_detail(ast->midcond);
                appcode(
                    CMP(value(0), nbyte_reg(ast->midcond->type->nbytes, reg)));
                appcode(JE(codeenv->break_label));
                restore_temp_reg(reg);
            }
            generate_register_code_detail(ast->for_body);
            appcode_str("%s:", codeenv->continue_label);
            if (ast->iterer != NULL) {
                int reg = generate_register_code_detail(ast->iterer);
                if (reg != -1) restore_temp_reg(reg);  // if nop
            }
            appcode(JMP(start_label));
            appcode_str("%s:", codeenv->break_label);

            RESTORE_BREAK_CXT;
            RESTORE_CONTINUE_CXT;

            return -1;
        }

        case AST_LABEL:
            appcode_str("%s:", ast->label_name);
            generate_register_code_detail(ast->label_stmt);
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

            int reg = generate_register_code_detail(ast->stsrc);
            appcode(ADD(value(offset), nbyte_reg(8, reg)));
            return reg;
        }

        case AST_COMPOUND:
            for (int i = 0; i < vector_size(ast->stmts); i++)
                generate_register_code_detail((AST *)vector_get(ast->stmts, i));
            return -1;

        case AST_EXPR_STMT:
            assert(temp_reg_table == 0);
            generate_basic_block_start_maker();
            if (ast->lhs != NULL) {
                int reg = generate_register_code_detail(ast->lhs);
                if (reg != -1) restore_temp_reg(reg);
            }
            assert(temp_reg_table == 0);
            generate_basic_block_end_marker();
            return -1;

        case AST_ARY2PTR:
            return generate_register_code_detail(ast->ary);

        case AST_CHAR2INT:
            return generate_register_code_detail(ast->lhs);

        case AST_LVALUE2RVALUE: {
            int lreg = generate_register_code_detail(ast->lhs),
                rreg = get_temp_reg();
            generate_mov_mem_reg(ast->type->nbytes, lreg, rreg);
            restore_temp_reg(lreg);
            return rreg;
        }

        case AST_LVAR_DECL_INIT:
        case AST_GVAR_DECL_INIT: {
            generate_register_code_detail(ast->lhs);
            int rreg = generate_register_code_detail(ast->rhs);
            if (rreg != -1) restore_temp_reg(rreg);
            return -1;
        }

        case AST_DECL_LIST:
            for (int i = 0; i < vector_size(ast->decls); i++)
                generate_register_code_detail((AST *)vector_get(ast->decls, i));
            return -1;

        case AST_VA_START: {
            int reg = generate_register_code_detail(ast->lhs);
            Code *reg_code = nbyte_reg(8, reg);
            assert(ast->rhs->kind == AST_INT);
            appcode(new_other_code("movl", value(ast->rhs->ival * 8),
                                   addrof(reg_code, 0)));
            appcode(new_other_code("movl", value(48), addrof(reg_code, 4)));
            appcode(LEA(addrof(RBP(), codeenv->overflow_arg_area_stack_idx),
                        RDI()));
            appcode(MOV(RDI(), addrof(reg_code, 8)));
            appcode(
                LEA(addrof(RBP(), codeenv->reg_save_area_stack_idx), RDI()));
            appcode(MOV(RDI(), addrof(reg_code, 16)));
            restore_temp_reg(reg);
            return -1;
        }

        case AST_GVAR_DECL:
        case AST_LVAR_DECL:
        case AST_FUNC_DECL:
        case AST_NOP:
            return -1;
    }

    warn("%d\n", ast->kind);
    assert(0);
}

Vector *generate_register_code(Vector *asts)
{
    init_code_env();

    appcode_str(".text");

    for (int i = 0; i < vector_size(asts); i++)
        generate_register_code_detail((AST *)vector_get(asts, i));

    appcode_str(".data");

    Vector *gvar_list = get_gvar_list();
    for (int i = 0; i < vector_size(gvar_list); i++) {
        GVar *gvar = (GVar *)vector_get(gvar_list, i);

        appcode_str("%s:", gvar->name);
        if (gvar->sval) {
            assert(gvar->type->kind == TY_ARY &&
                   gvar->type->ptr_of->kind == TY_CHAR);
            appcode_str(".ascii \"%s\"",
                        escape_string(gvar->sval, gvar->type->nbytes));
            continue;
        }

        if (gvar->ival == 0) {
            appcode_str(".zero %d", gvar->type->nbytes);
            continue;
        }

        assert(gvar->type->kind != TY_ARY);  // TODO: implement

        const char *type2spec[16];  // TODO: enough length?
        type2spec[TY_INT] = ".long";
        type2spec[TY_CHAR] = ".byte";
        type2spec[TY_PTR] = ".quad";
        appcode_str("%s %d", type2spec[gvar->type->kind], gvar->ival);
    }

    return clone_vector(codeenv->code);
}
