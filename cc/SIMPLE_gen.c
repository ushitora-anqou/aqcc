#include "cc.h"

struct SIMPLECode {
    enum {
        INST_MOV,
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

static SIMPLECode *MOV(SIMPLECode *lhs, SIMPLECode *rhs)
{
    SIMPLECode *code = new_code(INST_MOV);
    code->lhs = lhs;
    code->rhs = rhs;
    return code;
}

static SIMPLECode *RET() { return new_code(INST_RET); }

static SIMPLECode *HLT() { return new_code(INST_HLT); }

static SIMPLECode *CALL(char *label)
{
    SIMPLECode *code = new_code(INST_CALL);
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
        case INST_HLT:
            return "HLT";
        case INST_CALL:
            return format("CALL %s", code->label);
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

int SIMPLE_generate_code_detail(AST *ast)
{
    assert(ast != NULL);

    switch (ast->kind) {
        case AST_INT: {
            int regidx = get_temp_reg();
            appcode(MOV(reg(regidx), value(ast->ival)));
            return regidx;
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

    assert(0);
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
