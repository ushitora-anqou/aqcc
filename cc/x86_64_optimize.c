#include "cc.h"

AST *x86_64_optimize_ast_constant_detail(AST *ast, Env *env)
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

AST *x86_64_optimize_ast_constant(AST *ast, Env *env)
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

int get_using_register(Code *code)
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

int is_addrof_code(Code *code)
{
    if (code == NULL) return 0;
    return code->kind == CD_ADDR_OF || code->kind == CD_ADDR_OF_LABEL;
}

int is_same_code(Code *lhs, Code *rhs)
{
    if (lhs == NULL || rhs == NULL) return lhs == rhs;
    if (lhs->kind != rhs->kind) return 0;
    if (!is_same_code(lhs->lhs, rhs->lhs) || !is_same_code(lhs->rhs, rhs->rhs))
        return 0;
    if (lhs->ival != rhs->ival) return 0;
    if (lhs->label != rhs->label) return 0;  // TODO: strcmp
    return 1;
}

Vector *x86_64_optimize_code_detail_propagation(Vector *block)
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

Vector *x86_64_optimize_code_detail_eliminate(Vector *block)
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

int are_different_vectors(Vector *lhs, Vector *rhs)
{
    if (lhs == NULL || rhs == NULL) return lhs != rhs;
    int size = vector_size(lhs);
    if (size != vector_size(rhs)) return 1;
    for (int i = 0; i < size; i++)
        if (vector_get(lhs, i) != vector_get(rhs, i)) return 1;
    return 0;
}

Vector *x86_64_optimize_code_detail_basic_block(Vector *block)
{
    Vector *org_block = block, *nblock = block;
    do {
        org_block = nblock;
        nblock = x86_64_optimize_code_detail_propagation(nblock);
        nblock = x86_64_optimize_code_detail_eliminate(nblock);
    } while (are_different_vectors(org_block, nblock));  // do-while

    return nblock;
}

int x86_64_optimize_code_detail_funcdef(Vector *scode, int index, Vector *ncode)
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
