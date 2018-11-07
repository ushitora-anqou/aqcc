#include "cc.h"

struct SIMPLECode {
    enum {
        INST_MOV,
        INST_ADD,
        INST_SUB,
        INST_XOR,
        INST_SLL,
        INST_CMP,
        INST_JL,
        INST_JLE,
        INST_JMP,
        INST_HLT,
        INST_RET,
        INST_LABEL,
        INST_CALL,

        REG_R0,
        REG_R1,
        REG_R2,
        REG_R3,
        REG_R4,
        REG_R5,
        REG_R6,
        REG_R7,
        REG_SP,

        CD_VALUE,
    } kind;

    union {
        struct {
            SIMPLECode *lhs, *rhs;
        };
        int ival;
        char *label;
    };
};

static SIMPLECode *new_code(int kind)
{
    SIMPLECode *code = safe_malloc(sizeof(SIMPLECode));
    code->kind = kind;
    return code;
}

static SIMPLECode *new_binop_code(int kind, SIMPLECode *lhs, SIMPLECode *rhs)
{
    SIMPLECode *code = new_code(kind);
    code->lhs = lhs;
    code->rhs = rhs;
    return code;
}

static SIMPLECode *MOV(SIMPLECode *lhs, SIMPLECode *rhs)
{
    return new_binop_code(INST_MOV, lhs, rhs);
}

static SIMPLECode *ADD(SIMPLECode *lhs, SIMPLECode *rhs)
{
    return new_binop_code(INST_ADD, lhs, rhs);
}

static SIMPLECode *SUB(SIMPLECode *lhs, SIMPLECode *rhs)
{
    return new_binop_code(INST_SUB, lhs, rhs);
}

static SIMPLECode *XOR(SIMPLECode *lhs, SIMPLECode *rhs)
{
    return new_binop_code(INST_XOR, lhs, rhs);
}

static SIMPLECode *SLL(SIMPLECode *lhs, SIMPLECode *rhs)
{
    return new_binop_code(INST_SLL, lhs, rhs);
}

static SIMPLECode *CMP(SIMPLECode *lhs, SIMPLECode *rhs)
{
    return new_binop_code(INST_CMP, lhs, rhs);
}

static SIMPLECode *RET() { return new_code(INST_RET); }

static SIMPLECode *HLT() { return new_code(INST_HLT); }

static SIMPLECode *CALL(char *label)
{
    SIMPLECode *code = new_code(INST_CALL);
    code->label = label;
    return code;
}

static SIMPLECode *JL(char *label)
{
    SIMPLECode *code = new_code(INST_JL);
    code->label = label;
    return code;
}

static SIMPLECode *JLE(char *label)
{
    SIMPLECode *code = new_code(INST_JLE);
    code->label = label;
    return code;
}

static SIMPLECode *JMP(char *label)
{
    SIMPLECode *code = new_code(INST_JMP);
    code->label = label;
    return code;
}

static SIMPLECode *LABEL(char *label)
{
    SIMPLECode *code = new_code(INST_LABEL);
    code->label = label;
    return code;
}

static SIMPLECode *R0() { return new_code(REG_R0); }

static SIMPLECode *R1() { return new_code(REG_R1); }

static SIMPLECode *R2() { return new_code(REG_R2); }

static SIMPLECode *R3() { return new_code(REG_R3); }

static SIMPLECode *R4() { return new_code(REG_R4); }

static SIMPLECode *R5() { return new_code(REG_R5); }

static SIMPLECode *R6() { return new_code(REG_R6); }

static SIMPLECode *R7() { return new_code(REG_R7); }

static SIMPLECode *SP() { return new_code(REG_SP); }

static SIMPLECode *reg(int n)
{
    assert(0 <= n && n < 8);
    return new_code(REG_R0 + n);
}

static SIMPLECode *value(int value)
{
    SIMPLECode *code = new_code(CD_VALUE);
    code->ival = value;
    return code;
}

static int temp_reg_table;

static void init_temp_reg() { temp_reg_table = 0; }

static int get_temp_reg()
{
    for (int i = 0; i < 6; i++) {
        if (temp_reg_table & (1 << i)) continue;
        temp_reg_table |= (1 << i);
        return i;
    }

    error("no more register");
}

static void restore_temp_reg(int i) { temp_reg_table &= ~(1 << i); }

static char *code2str(SIMPLECode *code)
{
    if (code == NULL) return NULL;

    switch (code->kind) {
        case INST_MOV:
            return format("MOV %s, %s", code2str(code->lhs),
                          code2str(code->rhs));
        case INST_ADD:
            return format("ADD %s, %s", code2str(code->lhs),
                          code2str(code->rhs));
        case INST_SUB:
            return format("SUB %s, %s", code2str(code->lhs),
                          code2str(code->rhs));
        case INST_XOR:
            return format("XOR %s, %s", code2str(code->lhs),
                          code2str(code->rhs));
        case INST_SLL:
            return format("SLL %s, %s", code2str(code->lhs),
                          code2str(code->rhs));
        case INST_CMP:
            return format("CMP %s, %s", code2str(code->lhs),
                          code2str(code->rhs));
        case INST_HLT:
            return "HLT";
        case INST_CALL:
            return format("CALL %s", code->label);
        case INST_JMP:
            return format("JMP %s", code->label);
        case INST_JL:
            return format("JL %s", code->label);
        case INST_JLE:
            return format("JLE %s", code->label);
        case INST_RET:
            return "RET";
        case INST_LABEL:
            return format("%s:", code->label);
        case REG_R0:
            return "R0";
        case REG_R1:
            return "R1";
        case REG_R2:
            return "R2";
        case REG_R3:
            return "R3";
        case REG_R4:
            return "R4";
        case REG_R5:
            return "R5";
        case REG_R6:
            return "R6";
        case REG_R7:
            return "R7";
        case REG_SP:
            return "SP";
        case CD_VALUE:
            return format("%d", code->ival);
    }

    assert(0);
}

typedef struct {
    char *continue_label, *break_label;
    int reg_save_area_stack_idx, overflow_arg_area_stack_idx;
    Vector *code;
} CodeEnv;
CodeEnv *codeenv;

static void init_code_env()
{
    codeenv = (CodeEnv *)safe_malloc(sizeof(CodeEnv));
    codeenv->continue_label = codeenv->break_label = NULL;
    codeenv->reg_save_area_stack_idx = 0;
    codeenv->code = new_vector();
}

static void appcode(SIMPLECode *code) { vector_push_back(codeenv->code, code); }

static void generate_0xffff(int regi)
{
    appcode(MOV(reg(regi), value(0xff)));
    appcode(SLL(reg(regi), value(8)));
    appcode(MOV(reg(regi), value(0xff)));
}

static void generate_NOT(int srcreg)
{
    int tmpreg = get_temp_reg();
    generate_0xffff(tmpreg);
    appcode(XOR(reg(srcreg), reg(tmpreg)));
    restore_temp_reg(tmpreg);
}

int SIMPLE_generate_code_detail(AST *ast)
{
    assert(ast != NULL);

    switch (ast->kind) {
        case AST_INT: {
            int regidx = get_temp_reg();
            appcode(MOV(reg(regidx), value(ast->ival)));
            return regidx;
        }

        case AST_ADD: {
            int lreg = SIMPLE_generate_code_detail(ast->lhs),
                rreg = SIMPLE_generate_code_detail(ast->rhs);

            appcode(ADD(reg(lreg), reg(rreg)));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_SUB: {
            int lreg = SIMPLE_generate_code_detail(ast->lhs),
                rreg = SIMPLE_generate_code_detail(ast->rhs);

            appcode(SUB(reg(lreg), reg(rreg)));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_UNARY_MINUS: {
            int srcreg = SIMPLE_generate_code_detail(ast->lhs);
            generate_NOT(srcreg);
            int tmpreg = get_temp_reg();
            appcode(MOV(reg(tmpreg), value(1)));
            appcode(ADD(reg(srcreg), reg(tmpreg)));
            restore_temp_reg(tmpreg);
            return srcreg;
        }

        case AST_COMPL: {
            int srcreg = SIMPLE_generate_code_detail(ast->lhs);
            generate_NOT(srcreg);
            return srcreg;
        }

        case AST_LT: {
            int lreg = SIMPLE_generate_code_detail(ast->lhs),
                rreg = SIMPLE_generate_code_detail(ast->rhs);
            char *true_label = make_label_string(),
                 *exit_label = make_label_string();
            appcode(CMP(reg(lreg), reg(rreg)));
            appcode(JL(true_label));
            appcode(MOV(reg(lreg), value(0)));
            appcode(JMP(exit_label));
            appcode(LABEL(true_label));
            appcode(MOV(reg(lreg), value(1)));
            appcode(LABEL(exit_label));
            restore_temp_reg(rreg);
            return lreg;
        }

        case AST_LTE: {
            int lreg = SIMPLE_generate_code_detail(ast->lhs),
                rreg = SIMPLE_generate_code_detail(ast->rhs);
            char *true_label = make_label_string(),
                 *exit_label = make_label_string();
            appcode(CMP(reg(lreg), reg(rreg)));
            appcode(JLE(true_label));
            appcode(MOV(reg(lreg), value(0)));
            appcode(JMP(exit_label));
            appcode(LABEL(true_label));
            appcode(MOV(reg(lreg), value(1)));
            appcode(LABEL(exit_label));
            restore_temp_reg(rreg);
            return lreg;
        }
        case AST_RETURN:
            assert(temp_reg_table == 0);

            if (ast->lhs) {
                int regidx = SIMPLE_generate_code_detail(ast->lhs);
                appcode(MOV(R0(), reg(regidx)));
                restore_temp_reg(regidx);
            }
            else {
                appcode(MOV(R0(), value(0)));
            }
            appcode(RET());
            return -1;

        case AST_COMPOUND:
            for (int i = 0; i < vector_size(ast->stmts); i++)
                SIMPLE_generate_code_detail((AST *)vector_get(ast->stmts, i));
            return -1;

        case AST_FUNCDEF:
            appcode(LABEL(ast->fname));
            SIMPLE_generate_code_detail(ast->body);
            appcode(RET());
            return -1;
    }

    error("Unimplemented operation: %d", ast->kind);
}

Vector *SIMPLE_generate_code(Vector *asts)
{
    init_code_env();
    init_temp_reg();

    appcode(CALL("main"));
    appcode(HLT());

    for (int i = 0; i < vector_size(asts); i++) {
        AST *ast = (AST *)vector_get(asts, i);
        SIMPLE_generate_code_detail(ast);
    }

    return clone_vector(codeenv->code);
}

void SIMPLE_dump_code(SIMPLECode *code, FILE *fh)
{
    char *str = code2str(code);
    if (str != NULL) fprintf(fh, "%s\n", str);
    return;
}
