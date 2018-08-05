#include "aqcc.h"

int min(int a, int b) { return a < b ? a : b; }

#define SAVE_BREAK_CXT char *break_cxt__org_break_label = env->break_label;
#define RESTORE_BREAK_CXT env->break_label = break_cxt__org_break_label;
#define SAVE_CONTINUE_CXT \
    char *continue_cxt__org_continue_label = env->continue_label;
#define RESTORE_CONTINUE_CXT \
    env->continue_label = continue_cxt__org_continue_label;

typedef struct {
    char *continue_label, *break_label;
    Vector *codes;
} CodeEnv;

void generate_code_detail(CodeEnv *env, AST *ast);

CodeEnv *new_code_env()
{
    CodeEnv *this;

    this = (CodeEnv *)safe_malloc(sizeof(CodeEnv));
    this->continue_label = this->break_label = NULL;
    this->codes = new_vector();
    return this;
}

Vector *swap_codes(CodeEnv *env, Vector *new_codes)
{
    Vector *ret = env->codes;
    env->codes = new_codes;
    return ret;
}

void appcode(Vector *codes, const char *src, ...)
{
    char buf[256], buf2[256];  // TODO: enoguth length?
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

    vector_push_back(codes, new_str(buf2));
}

void dump_codes(Vector *codes, FILE *fh)
{
    int i;

    for (i = 0; i < vector_size(codes); i++)
        fprintf(fh, "%s\n", (const char *)vector_get(codes, i));
}

void generate_mov_from_memory(CodeEnv *env, int nbytes, const char *src,
                              int dst_reg)
{
    switch (nbytes) {
        case 1:
            appcode(env->codes, "movsbl %s, %s", src, reg_name(4, dst_reg));
            break;

        case 4:
        case 8:
            appcode(env->codes, "mov %s, %s", src, reg_name(nbytes, dst_reg));
            break;
    }
}

void generate_code_detail_lvalue(CodeEnv *env, AST *ast)
{
    switch (ast->kind) {
        case AST_LVAR:
            appcode(env->codes, "lea %d(#rbp), #rax", ast->stack_idx);
            appcode(env->codes, "push #rax");
            break;

        case AST_GVAR:
            appcode(env->codes, "lea %s(#rip), #rax", ast->varname);
            appcode(env->codes, "push #rax");
            break;

        case AST_INDIR:
            generate_code_detail(env, ast->lhs);
            break;

        default:
            assert(0);
    }
}

void generate_code_detail(CodeEnv *env, AST *ast)
{
    switch (ast->kind) {
        case AST_ADD:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");

            // int + ptr
            if (match_type2(ast->lhs, ast->rhs, TY_INT, TY_PTR)) {
                appcode(env->codes, "cltq");  // TODO: long
                appcode(env->codes, "imul $%d, #rax",
                        ast->rhs->type->ptr_of->nbytes);
            }

            appcode(env->codes, "add %s, %s", reg_name(ast->type->nbytes, 1),
                    reg_name(ast->type->nbytes, 0));
            appcode(env->codes, "push #rax");
            break;

        case AST_SUB: {
            int nbytes;

            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);

            appcode(env->codes, "pop #rax");
            appcode(env->codes, "pop #rdi");

            // ptr - int
            if (match_type2(ast->lhs, ast->rhs, TY_PTR, TY_INT)) {
                appcode(env->codes, "cltq");  // TODO: long
                appcode(env->codes, "imul $%d, #rax",
                        ast->lhs->type->ptr_of->nbytes);
            }

            nbytes = ast->lhs->type->nbytes;
            appcode(env->codes, "sub %s, %s", reg_name(nbytes, 0),
                    reg_name(nbytes, 1));

            // ptr - ptr
            if (match_type2(ast->lhs, ast->rhs, TY_PTR, TY_PTR))
                // TODO: assume pointer size is 8.
                appcode(env->codes, "sar $%d, #rdi", 2);

            appcode(env->codes, "push #rdi");
        } break;

        case AST_MUL:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "imul #edi, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_DIV:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cltd");
            appcode(env->codes, "idiv #edi");
            appcode(env->codes, "push #rax");
            break;

        case AST_REM:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cltd");
            appcode(env->codes, "idiv #edi");
            appcode(env->codes, "push #rdx");
            break;

        case AST_UNARY_MINUS:
            generate_code_detail(env, ast->lhs);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "neg #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_LSHIFT:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rcx");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "sal #cl, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_RSHIFT:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rcx");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "sar #cl, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_LT:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp #edi, #eax");
            appcode(env->codes, "setl #al");
            appcode(env->codes, "movzb #al, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_LTE:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp #edi, #eax");
            appcode(env->codes, "setle #al");
            appcode(env->codes, "movzb #al, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_EQ:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp #edi, #eax");
            appcode(env->codes, "sete #al");
            appcode(env->codes, "movzb #al, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_NEQ:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp #edi, #eax");
            appcode(env->codes, "setne #al");
            appcode(env->codes, "movzb #al, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_AND:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "and #edi, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_XOR:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "xor #edi, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_OR:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "or #edi, #eax");
            appcode(env->codes, "push #rax");
            break;

        case AST_LAND: {
            char *false_label = make_label_string(),
                 *exit_label = make_label_string();
            generate_code_detail(env, ast->lhs);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp $0, #eax");
            appcode(env->codes, "je %s", false_label);
            // don't execute rhs expression if lhs is false.
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp $0, #eax");
            appcode(env->codes, "je %s", false_label);
            appcode(env->codes, "mov $1, #eax");
            appcode(env->codes, "jmp %s", exit_label);
            appcode(env->codes, "%s:", false_label);
            appcode(env->codes, "mov $0, #eax");
            appcode(env->codes, "%s:", exit_label);
            appcode(env->codes, "push #rax");
        } break;

        case AST_LOR: {
            char *true_label = make_label_string(),
                 *exit_label = make_label_string();
            generate_code_detail(env, ast->lhs);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp $0, #eax");
            appcode(env->codes, "jne %s", true_label);
            // don't execute rhs expression if lhs is true.
            generate_code_detail(env, ast->rhs);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp $0, #eax");
            appcode(env->codes, "jne %s", true_label);
            appcode(env->codes, "mov $0, #eax");
            appcode(env->codes, "jmp %s", exit_label);
            appcode(env->codes, "%s:", true_label);
            appcode(env->codes, "mov $1, #eax");
            appcode(env->codes, "%s:", exit_label);
            appcode(env->codes, "push #rax");
        } break;

        case AST_POSTINC: {
            char suf;

            suf = byte2suffix(ast->lhs->type->nbytes);

            generate_code_detail_lvalue(env, ast->lhs);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "push (#rax)");

            if (match_type(ast->lhs, TY_PTR))
                appcode(env->codes, "add%c $%d, (#rax)", suf,
                        ast->lhs->type->ptr_of->nbytes);
            else
                appcode(env->codes, "inc%c (#rax)", suf);

        } break;

        case AST_PREINC: {
            char suf;

            suf = byte2suffix(ast->lhs->type->nbytes);

            generate_code_detail_lvalue(env, ast->lhs);
            appcode(env->codes, "pop #rax");

            if (match_type(ast->lhs, TY_PTR))
                appcode(env->codes, "add%c $%d, (#rax)", suf,
                        ast->lhs->type->ptr_of->nbytes);
            else
                appcode(env->codes, "inc%c (#rax)", suf);

            appcode(env->codes, "push (#rax)");
        } break;

        case AST_ADDR:
            generate_code_detail_lvalue(env, ast->lhs);
            break;

        case AST_INDIR: {
            generate_code_detail(env, ast->lhs);
            appcode(env->codes, "pop #rax");
            generate_mov_from_memory(env, ast->type->nbytes, "(%rax)", 1);
            appcode(env->codes, "push #rdi");
        } break;

        case AST_IF:
        case AST_COND: {
            char *false_label = make_label_string(),
                 *exit_label = make_label_string();

            generate_code_detail(env, ast->cond);
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "cmp $0, #eax");
            appcode(env->codes, "je %s", false_label);
            generate_code_detail(env, ast->then);
            appcode(env->codes, "jmp %s", exit_label);
            appcode(env->codes, "%s:", false_label);
            if (ast->els != NULL) generate_code_detail(env, ast->els);
            appcode(env->codes, "%s:", exit_label);
        } break;

        case AST_SWITCH: {
            generate_code_detail(env, ast->target);
            appcode(env->codes, "pop #rax");

            for (int i = 0; i < vector_size(ast->cases); i++) {
                SwitchCase *cas = (SwitchCase *)vector_get(ast->cases, i);
                appcode(env->codes, "cmp $%d, #eax", cas->cond);
                // case has been already labeled when analyzing.
                appcode(env->codes, "je %s", cas->label_name);
            }
            char *exit_label = make_label_string();
            if (ast->default_label)
                appcode(env->codes, "jmp %s", ast->default_label);
            else
                appcode(env->codes, "jmp %s", exit_label);

            SAVE_BREAK_CXT;
            env->break_label = exit_label;
            generate_code_detail(env, ast->switch_body);
            appcode(env->codes, "%s:", exit_label);
            RESTORE_BREAK_CXT;
        } break;

        case AST_LABEL:
            appcode(env->codes, "%s:", ast->label_name);
            generate_code_detail(env, ast->label_stmt);
            break;

        case AST_ASSIGN:
            generate_code_detail(env, ast->rhs);
            generate_code_detail_lvalue(env, ast->lhs);

            appcode(env->codes, "pop #rdi");
            appcode(env->codes, "pop #rax");
            appcode(env->codes, "mov %s, (#rdi)",
                    reg_name(ast->lhs->type->nbytes, 0));
            appcode(env->codes, "push #rax");
            break;

        case AST_LVAR:
            generate_mov_from_memory(env, ast->type->nbytes,
                                     format("%d(%rbp)", ast->stack_idx), 0);
            appcode(env->codes, "push #rax");
            break;

        case AST_GVAR:
            generate_mov_from_memory(env, ast->type->nbytes,
                                     format("%s(%rip)", ast->varname), 0);
            appcode(env->codes, "push #rax");
            break;

        case AST_FUNCCALL:
            for (int i = vector_size(ast->args) - 1; i >= 0; i--)
                generate_code_detail(env, (AST *)(vector_get(ast->args, i)));
            for (int i = 0; i < min(6, vector_size(ast->args)); i++)
                appcode(env->codes, "pop %s", reg_name(8, i + 1));
            appcode(env->codes, "mov $0, #eax");
            appcode(env->codes, "call %s@PLT", ast->fname);
            appcode(env->codes, "push #rax");
            break;

        case AST_FUNCDEF: {
            int i, stack_idx;

            // allocate stack
            stack_idx = 0;
            for (i = max(0, vector_size(ast->params) - 6);
                 i < vector_size(ast->env->scoped_vars); i++) {
                AST *var = (AST *)(vector_get(ast->env->scoped_vars, i));

                stack_idx -= var->type->nbytes;
                var->stack_idx = stack_idx;
            }

            // generate code
            appcode(env->codes, "%s:", ast->fname);
            appcode(env->codes, "push #rbp");
            appcode(env->codes, "mov #rsp, #rbp");
            appcode(env->codes, "sub $%d, #rsp",
                    (int)(ceil(-stack_idx / 16.)) * 16);

            // assign param to localvar
            for (i = 0; i < vector_size(ast->params); i++) {
                AST *var = lookup_var(
                    ast->env, ((AST *)vector_get(ast->params, i))->varname);
                if (i < 6)
                    appcode(env->codes, "mov %s, %d(#rbp)",
                            reg_name(var->type->nbytes, i + 1), var->stack_idx);
                else
                    // should avoid return pointer and saved %rbp
                    var->stack_idx = 16 + (i - 6) * 8;
            }

            // generate body
            generate_code_detail(env, ast->body);

            // avoid duplicate needless `ret`
            if (strcmp((const char *)vector_get(env->codes,
                                                vector_size(env->codes) - 1),
                       "ret") == 0)
                break;
            appcode(env->codes, "mov $0, #eax");
            appcode(env->codes, "mov #rbp, #rsp");
            appcode(env->codes, "pop #rbp");
            appcode(env->codes, "ret");
        } break;

        case AST_EXPR_STMT:
            if (ast->lhs == NULL) break;
            generate_code_detail(env, ast->lhs);
            appcode(env->codes, "pop #rax");
            break;

        case AST_RETURN:
            if (ast->lhs == NULL) {
                appcode(env->codes, "mov $0, #eax");
            }
            else {
                generate_code_detail(env, ast->lhs);
                appcode(env->codes, "pop #rax");
            }
            appcode(env->codes, "mov #rbp, #rsp");
            appcode(env->codes, "pop #rbp");
            appcode(env->codes, "ret");
            break;

        case AST_FOR: {
            SAVE_BREAK_CXT;
            SAVE_CONTINUE_CXT;
            env->break_label = make_label_string();
            env->continue_label = make_label_string();
            char *start_label = make_label_string();

            if (ast->initer != NULL) {
                generate_code_detail(env, ast->initer);
                appcode(env->codes, "pop #rax");
            }
            appcode(env->codes, "%s:", start_label);
            if (ast->midcond != NULL) {
                generate_code_detail(env, ast->midcond);
                appcode(env->codes, "pop #rax");
                appcode(env->codes, "cmp $0, #eax");
                appcode(env->codes, "je %s", env->break_label);
            }
            generate_code_detail(env, ast->for_body);
            appcode(env->codes, "%s:", env->continue_label);
            if (ast->iterer != NULL) {
                generate_code_detail(env, ast->iterer);
                appcode(env->codes, "pop #rax");
            }
            appcode(env->codes, "jmp %s", start_label);
            appcode(env->codes, "%s:", env->break_label);

            RESTORE_BREAK_CXT;
            RESTORE_CONTINUE_CXT;
        } break;

        case AST_BREAK:
            if (env->break_label < 0)
                error("invalid break.", __FILE__, __LINE__);
            appcode(env->codes, "jmp %s", env->break_label);
            break;

        case AST_CONTINUE:
            if (env->continue_label < 0)
                error("invalid continue.", __FILE__, __LINE__);
            appcode(env->codes, "jmp %s", env->continue_label);
            break;

        case AST_GOTO:
            appcode(env->codes, "jmp %s", ast->label_name);
            break;

        case AST_COMPOUND: {
            int i;

            for (i = 0; i < vector_size(ast->stmts); i++)
                generate_code_detail(env, (AST *)vector_get(ast->stmts, i));
        } break;

        case AST_INT:
            appcode(env->codes, "mov $%d, #eax", ast->ival);
            appcode(env->codes, "push #rax");
            break;

        case AST_ARY2PTR:
            // TODO: is it always safe to treat ary2ptr as lvalue generation?
            generate_code_detail_lvalue(env, ast->ary);
            break;

        case AST_CHAR2INT:
            generate_code_detail(env, ast->lhs);
            break;

        case AST_LVAR_DECL_INIT:
        case AST_GVAR_DECL_INIT:
            generate_code_detail(env, ast->lhs);
            generate_code_detail(env, ast->rhs);
            break;

        case AST_GVAR_DECL:
        case AST_LVAR_DECL:
        case AST_FUNC_DECL:
        case AST_NOP:
            break;

        default:
            assert(0);
    }
}

Vector *generate_code(Vector *asts)
{
    CodeEnv *env;

    env = new_code_env();

    appcode(env->codes, ".global main");
    appcode(env->codes, ".text");

    for (int i = 0; i < vector_size(asts); i++)
        generate_code_detail(env, (AST *)vector_get(asts, i));

    appcode(env->codes, ".data");

    Vector *gvar_list = get_gvar_list();
    for (int i = 0; i < vector_size(gvar_list); i++) {
        GVar *gvar = (GVar *)vector_get(gvar_list, i);

        appcode(env->codes, "%s:", gvar->name);
        if (gvar->sval) {
            assert(gvar->type->kind == TY_ARY &&
                   gvar->type->ptr_of->kind == TY_CHAR);
            appcode(env->codes, ".ascii \"%s\"",
                    escape_string(gvar->sval, gvar->type->nbytes));
            continue;
        }

        if (gvar->ival == 0) {
            appcode(env->codes, ".zero %d", gvar->type->nbytes);
            continue;
        }

        assert(gvar->type->kind != TY_ARY);  // TODO: implement

        const char *type2spec[16];  // TODO: enough length?
        type2spec[TY_INT] = ".long";
        type2spec[TY_CHAR] = ".byte";
        type2spec[TY_PTR] = ".quad";
        appcode(env->codes, "%s %d", type2spec[gvar->type->kind], gvar->ival);
    }

    return env->codes;
}
