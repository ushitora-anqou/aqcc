#include "aqcc.h"

#define SAVE_BREAK_CXT char *break_cxt__org_break_label = env->break_label;
#define RESTORE_BREAK_CXT env->break_label = break_cxt__org_break_label;
#define SAVE_CONTINUE_CXT \
    char *continue_cxt__org_continue_label = env->continue_label;
#define RESTORE_CONTINUE_CXT \
    env->continue_label = continue_cxt__org_continue_label;

const char *reg_name(int byte, int i)
{
    const char *lreg[] = {"%al", "%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
    const char *xreg[] = {"%ax", "%di", "%si", "%dx", "%cx", "%r8w", "%r9w"};
    const char *ereg[] = {"%eax", "%edi", "%esi", "%edx",
                          "%ecx", "%r8d", "%r9d"};
    const char *rreg[] = {"%rax", "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

    assert(0 <= i && i <= 6);

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
    Vector *code;
} CodeEnv;
CodeEnv *env;

void generate_code_detail(AST *ast);

void init_code_env()
{
    env = (CodeEnv *)safe_malloc(sizeof(CodeEnv));
    env->continue_label = env->break_label = NULL;
    env->code = new_vector();
}

void appcode(const char *src, ...)
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

    vector_push_back(env->code, new_str(buf2));
}

void dump_code(Vector *code, FILE *fh)
{
    for (int i = 0; i < vector_size(code); i++)
        fprintf(fh, "%s\n", (const char *)vector_get(code, i));
}

const char *last_appended_code()
{
    return (const char *)vector_get(env->code, vector_size(env->code) - 1);
}

void generate_mov_from_memory(int nbytes, const char *src, int dst_reg)
{
    switch (nbytes) {
        case 1:
            appcode("movsbl %s, %s", src, reg_name(4, dst_reg));
            break;

        case 4:
        case 8:
            appcode("mov %s, %s", src, reg_name(nbytes, dst_reg));
            break;

        default:
            assert(0);
    }
}

void generate_code_detail(AST *ast)
{
    switch (ast->kind) {
        case AST_ADD:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            appcode("pop #rdi");
            appcode("pop #rax");

            // int + ptr
            if (match_type2(ast->lhs, ast->rhs, TY_INT, TY_PTR)) {
                appcode("cltq");  // TODO: long
                appcode("imul $%d, #rax", ast->rhs->type->ptr_of->nbytes);
            }

            appcode("add %s, %s", reg_name(ast->type->nbytes, 1),
                    reg_name(ast->type->nbytes, 0));
            appcode("push #rax");
            break;

        case AST_SUB: {
            int nbytes;

            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);

            appcode("pop #rax");
            appcode("pop #rdi");

            // ptr - int
            if (match_type2(ast->lhs, ast->rhs, TY_PTR, TY_INT)) {
                appcode("cltq");  // TODO: long
                appcode("imul $%d, #rax", ast->lhs->type->ptr_of->nbytes);
            }

            nbytes = ast->lhs->type->nbytes;
            appcode("sub %s, %s", reg_name(nbytes, 0), reg_name(nbytes, 1));

            // ptr - ptr
            if (match_type2(ast->lhs, ast->rhs, TY_PTR, TY_PTR))
                // TODO: assume pointer size is 8.
                appcode("sar $%d, #rdi", 2);

            appcode("push #rdi");
        } break;

        case AST_MUL:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            appcode("pop #rdi");
            appcode("pop #rax");
            appcode("imul #edi, #eax");
            appcode("push #rax");
            break;

        case AST_DIV:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            appcode("pop #rdi");
            appcode("pop #rax");
            appcode("cltd");
            appcode("idiv #edi");
            appcode("push #rax");
            break;

        case AST_REM:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            appcode("pop #rdi");
            appcode("pop #rax");
            appcode("cltd");
            appcode("idiv #edi");
            appcode("push #rdx");
            break;

        case AST_UNARY_MINUS:
            generate_code_detail(ast->lhs);
            appcode("pop #rax");
            appcode("neg #eax");
            appcode("push #rax");
            break;

        case AST_LSHIFT:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            appcode("pop #rcx");
            appcode("pop #rax");
            appcode("sal #cl, #eax");
            appcode("push #rax");
            break;

        case AST_RSHIFT:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            appcode("pop #rcx");
            appcode("pop #rax");
            appcode("sar #cl, #eax");
            appcode("push #rax");
            break;

        case AST_LT:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            appcode("pop #rdi");
            appcode("pop #rax");
            appcode("cmp #edi, #eax");
            appcode("setl #al");
            appcode("movzb #al, #eax");
            appcode("push #rax");
            break;

        case AST_LTE:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            appcode("pop #rdi");
            appcode("pop #rax");
            appcode("cmp #edi, #eax");
            appcode("setle #al");
            appcode("movzb #al, #eax");
            appcode("push #rax");
            break;

        case AST_EQ:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            appcode("pop #rdi");
            appcode("pop #rax");
            appcode("cmp #edi, #eax");
            appcode("sete #al");
            appcode("movzb #al, #eax");
            appcode("push #rax");
            break;

        case AST_AND:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            appcode("pop #rdi");
            appcode("pop #rax");
            appcode("and #edi, #eax");
            appcode("push #rax");
            break;

        case AST_XOR:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            appcode("pop #rdi");
            appcode("pop #rax");
            appcode("xor #edi, #eax");
            appcode("push #rax");
            break;

        case AST_OR:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            appcode("pop #rdi");
            appcode("pop #rax");
            appcode("or #edi, #eax");
            appcode("push #rax");
            break;

        case AST_LAND: {
            char *false_label = make_label_string(),
                 *exit_label = make_label_string();
            generate_code_detail(ast->lhs);
            appcode("pop #rax");
            appcode("cmp $0, #eax");
            appcode("je %s", false_label);
            // don't execute rhs expression if lhs is false.
            generate_code_detail(ast->rhs);
            appcode("pop #rax");
            appcode("cmp $0, #eax");
            appcode("je %s", false_label);
            appcode("mov $1, #eax");
            appcode("jmp %s", exit_label);
            appcode("%s:", false_label);
            appcode("mov $0, #eax");
            appcode("%s:", exit_label);
            appcode("push #rax");
        } break;

        case AST_LOR: {
            char *true_label = make_label_string(),
                 *exit_label = make_label_string();
            generate_code_detail(ast->lhs);
            appcode("pop #rax");
            appcode("cmp $0, #eax");
            appcode("jne %s", true_label);
            // don't execute rhs expression if lhs is true.
            generate_code_detail(ast->rhs);
            appcode("pop #rax");
            appcode("cmp $0, #eax");
            appcode("jne %s", true_label);
            appcode("mov $0, #eax");
            appcode("jmp %s", exit_label);
            appcode("%s:", true_label);
            appcode("mov $1, #eax");
            appcode("%s:", exit_label);
            appcode("push #rax");
        } break;

        case AST_POSTINC: {
            char suf = byte2suffix(ast->lhs->type->nbytes);

            generate_code_detail(ast->lhs);
            appcode("pop #rax");
            appcode("push (#rax)");

            if (match_type(ast->lhs, TY_PTR))
                appcode("add%c $%d, (#rax)", suf,
                        ast->lhs->type->ptr_of->nbytes);
            else
                appcode("inc%c (#rax)", suf);

        } break;

        case AST_PREINC: {
            char suf = byte2suffix(ast->lhs->type->nbytes);

            generate_code_detail(ast->lhs);
            appcode("pop #rax");

            if (match_type(ast->lhs, TY_PTR))
                appcode("add%c $%d, (#rax)", suf,
                        ast->lhs->type->ptr_of->nbytes);
            else
                appcode("inc%c (#rax)", suf);

            appcode("push (#rax)");
        } break;

        case AST_POSTDEC: {
            char suf = byte2suffix(ast->lhs->type->nbytes);

            generate_code_detail(ast->lhs);
            appcode("pop #rax");
            appcode("push (#rax)");

            if (match_type(ast->lhs, TY_PTR))
                appcode("sub%c $%d, (#rax)", suf,
                        ast->lhs->type->ptr_of->nbytes);
            else
                appcode("dec%c (#rax)", suf);

        } break;

        case AST_PREDEC: {
            char suf = byte2suffix(ast->lhs->type->nbytes);

            generate_code_detail(ast->lhs);
            appcode("pop #rax");

            if (match_type(ast->lhs, TY_PTR))
                appcode("sub%c $%d, (#rax)", suf,
                        ast->lhs->type->ptr_of->nbytes);
            else
                appcode("dec%c (#rax)", suf);

            appcode("push (#rax)");
        } break;

        case AST_ADDR:
        case AST_INDIR:
            generate_code_detail(ast->lhs);
            break;

        case AST_IF:
        case AST_COND: {
            char *false_label = make_label_string(),
                 *exit_label = make_label_string();

            generate_code_detail(ast->cond);
            appcode("pop #rax");
            appcode("cmp $0, #eax");
            appcode("je %s", false_label);
            generate_code_detail(ast->then);
            appcode("jmp %s", exit_label);
            appcode("%s:", false_label);
            if (ast->els != NULL) generate_code_detail(ast->els);
            appcode("%s:", exit_label);
        } break;

        case AST_SWITCH: {
            generate_code_detail(ast->target);
            appcode("pop #rax");

            for (int i = 0; i < vector_size(ast->cases); i++) {
                SwitchCase *cas = (SwitchCase *)vector_get(ast->cases, i);
                appcode("cmp $%d, #eax", cas->cond);
                // case has been already labeled when analyzing.
                appcode("je %s", cas->label_name);
            }
            char *exit_label = make_label_string();
            if (ast->default_label)
                appcode("jmp %s", ast->default_label);
            else
                appcode("jmp %s", exit_label);

            SAVE_BREAK_CXT;
            env->break_label = exit_label;
            generate_code_detail(ast->switch_body);
            appcode("%s:", exit_label);
            RESTORE_BREAK_CXT;
        } break;

        case AST_LABEL:
            appcode("%s:", ast->label_name);
            generate_code_detail(ast->label_stmt);
            break;

        case AST_ASSIGN:
            generate_code_detail(ast->rhs);
            generate_code_detail(ast->lhs);

            appcode("pop #rdi");
            appcode("pop #rax");
            appcode("mov %s, (#rdi)", reg_name(ast->lhs->type->nbytes, 0));
            appcode("push #rax");
            break;

        case AST_EXPR_LIST:
            for (int i = 0; i < vector_size(ast->exprs); i++) {
                generate_code_detail((AST *)vector_get(ast->exprs, i));
                appcode("pop #rax");
            }
            appcode("push #rax");
            break;

        case AST_LVAR:
            if (ast->type->is_static || ast->type->is_extern)
                appcode("lea %s(#rip), #rax", ast->gen_varname);
            else
                appcode("lea %d(#rbp), #rax", ast->stack_idx);
            appcode("push #rax");
            break;

        case AST_GVAR:
            appcode("lea %s(#rip), #rax", ast->gen_varname);
            appcode("push #rax");
            break;

        case AST_FUNCCALL:
            for (int i = vector_size(ast->args) - 1; i >= 0; i--)
                generate_code_detail((AST *)(vector_get(ast->args, i)));
            for (int i = 0; i < min(6, vector_size(ast->args)); i++)
                appcode("pop %s", reg_name(8, i + 1));
            appcode("mov $0, #eax");
            appcode("call %s@PLT", ast->fname);
            appcode("push #rax");
            break;

        case AST_FUNCDEF: {
            // allocate stack
            int stack_idx = 0;
            for (int i = ast->params ? max(0, vector_size(ast->params) - 6) : 0;
                 i < vector_size(ast->env->scoped_vars); i++) {
                AST *var = (AST *)(vector_get(ast->env->scoped_vars, i));

                stack_idx -= var->type->nbytes;
                var->stack_idx = stack_idx;
            }

            // generate code
            appcode(".global %s", ast->fname);
            appcode("%s:", ast->fname);
            appcode("push #rbp");
            appcode("mov #rsp, #rbp");
            appcode("sub $%d, #rsp", roundup(-stack_idx, 16));

            // assign param to localvar
            if (ast->params) {
                for (int i = 0; i < vector_size(ast->params); i++) {
                    AST *var = lookup_var(
                        ast->env, ((AST *)vector_get(ast->params, i))->varname);
                    if (i < 6)
                        appcode("mov %s, %d(#rbp)",
                                reg_name(var->type->nbytes, i + 1),
                                var->stack_idx);
                    else
                        // should avoid return pointer and saved %rbp
                        var->stack_idx = 16 + (i - 6) * 8;
                }
            }

            // generate body
            generate_code_detail(ast->body);

            // avoid duplicate needless `ret`
            if (strcmp(last_appended_code(), "ret") == 0) break;

            if (ast->type->kind != TY_VOID) appcode("mov $0, #rax");
            appcode("mov #rbp, #rsp");
            appcode("pop #rbp");
            appcode("ret");
        } break;

        case AST_EXPR_STMT:
            if (ast->lhs == NULL) break;
            generate_code_detail(ast->lhs);
            appcode("pop #rax");
            break;

        case AST_RETURN:
            if (ast->lhs == NULL) {
                appcode("mov $0, #eax");
            }
            else {
                generate_code_detail(ast->lhs);
                appcode("pop #rax");
            }
            appcode("mov #rbp, #rsp");
            appcode("pop #rbp");
            appcode("ret");
            break;

        case AST_DOWHILE: {
            SAVE_BREAK_CXT;
            SAVE_CONTINUE_CXT;
            env->break_label = make_label_string();
            env->continue_label = make_label_string();
            char *start_label = make_label_string();

            appcode("%s:", start_label);
            generate_code_detail(ast->then);
            appcode("%s:", env->continue_label);
            generate_code_detail(ast->cond);
            appcode("pop #rax");
            appcode("cmp $0, #eax");
            appcode("jne %s", start_label);
            appcode("%s:", env->break_label);

            RESTORE_BREAK_CXT;
            RESTORE_CONTINUE_CXT;
        } break;

        case AST_FOR: {
            SAVE_BREAK_CXT;
            SAVE_CONTINUE_CXT;
            env->break_label = make_label_string();
            env->continue_label = make_label_string();
            char *start_label = make_label_string();

            if (ast->initer != NULL) {
                generate_code_detail(ast->initer);
                if (ast->type != NULL)  // if ast is expr
                    appcode("pop #rax");
            }
            appcode("%s:", start_label);
            if (ast->midcond != NULL) {
                generate_code_detail(ast->midcond);
                appcode("pop #rax");
                appcode("cmp $0, #eax");
                appcode("je %s", env->break_label);
            }
            generate_code_detail(ast->for_body);
            appcode("%s:", env->continue_label);
            if (ast->iterer != NULL) {
                generate_code_detail(ast->iterer);
                appcode("pop #rax");
            }
            appcode("jmp %s", start_label);
            appcode("%s:", env->break_label);

            RESTORE_BREAK_CXT;
            RESTORE_CONTINUE_CXT;
        } break;

        case AST_BREAK:
            if (env->break_label < 0) error("invalid break.");
            appcode("jmp %s", env->break_label);
            break;

        case AST_CONTINUE:
            if (env->continue_label < 0) error("invalid continue.");
            appcode("jmp %s", env->continue_label);
            break;

        case AST_GOTO:
            appcode("jmp %s", ast->label_name);
            break;

        case AST_MEMBER_REF: {
            int offset =
                lookup_member_offset(ast->stsrc->type->members, ast->member);
            // the member existence is confirmed when analysis.
            assert(offset >= 0);

            generate_code_detail(ast->stsrc);
            appcode("pop #rax");
            appcode("add $%d, #rax", offset);
            appcode("push #rax");
        } break;

        case AST_COMPOUND: {
            int i;

            for (i = 0; i < vector_size(ast->stmts); i++)
                generate_code_detail((AST *)vector_get(ast->stmts, i));
        } break;

        case AST_INT:
            appcode("mov $%d, #eax", ast->ival);
            appcode("push #rax");
            break;

        case AST_ARY2PTR:
            generate_code_detail(ast->ary);
            break;

        case AST_CHAR2INT:
            generate_code_detail(ast->lhs);
            break;

        case AST_LVALUE2RVALUE:
            generate_code_detail(ast->lhs);
            appcode("pop #rax");
            generate_mov_from_memory(ast->type->nbytes, "(%rax)", 1);
            appcode("push #rdi");
            break;

        case AST_LVAR_DECL_INIT:
        case AST_GVAR_DECL_INIT:
            generate_code_detail(ast->lhs);
            generate_code_detail(ast->rhs);
            if (ast->rhs->kind == AST_ASSIGN) appcode("pop #rax");
            break;

        case AST_DECL_LIST:
            for (int i = 0; i < vector_size(ast->decls); i++)
                generate_code_detail((AST *)vector_get(ast->decls, i));
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
    init_code_env();

    appcode(".text");

    for (int i = 0; i < vector_size(asts); i++)
        generate_code_detail((AST *)vector_get(asts, i));

    appcode(".data");

    Vector *gvar_list = get_gvar_list();
    for (int i = 0; i < vector_size(gvar_list); i++) {
        GVar *gvar = (GVar *)vector_get(gvar_list, i);

        appcode("%s:", gvar->name);
        if (gvar->sval) {
            assert(gvar->type->kind == TY_ARY &&
                   gvar->type->ptr_of->kind == TY_CHAR);
            appcode(".ascii \"%s\"",
                    escape_string(gvar->sval, gvar->type->nbytes));
            continue;
        }

        if (gvar->ival == 0) {
            appcode(".zero %d", gvar->type->nbytes);
            continue;
        }

        assert(gvar->type->kind != TY_ARY);  // TODO: implement

        const char *type2spec[16];  // TODO: enough length?
        type2spec[TY_INT] = ".long";
        type2spec[TY_CHAR] = ".byte";
        type2spec[TY_PTR] = ".quad";
        appcode("%s %d", type2spec[gvar->type->kind], gvar->ival);
    }

    return clone_vector(env->code);
}
