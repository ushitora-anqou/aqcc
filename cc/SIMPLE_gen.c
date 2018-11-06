#include "cc.h"

struct SIMPLECode {
    enum {
        INST_MOV,
        INST_HLT,

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

static SIMPLECode *HLT() { return new_code(INST_HLT); }

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

static char *code2str(SIMPLECode *code)
{
    if (code == NULL) return NULL;

    switch (code->kind) {
        case INST_MOV:
            return format("MOV %s, %s", code2str(code->lhs),
                          code2str(code->rhs));
        case INST_HLT:
            return "HLT";
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

Vector *SIMPLE_generate_code(Vector *asts)
{
    Vector *ret = new_vector();
    vector_push_back(ret, MOV(R0(), value(10)));
    vector_push_back(ret, HLT());
    return ret;
}

void SIMPLE_dump_code(SIMPLECode *code, FILE *fh)
{
    char *str = code2str(code);
    if (str != NULL) fprintf(fh, "%s\n", str);
    return;
}
