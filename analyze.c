#include "aqcc.h"

Vector *gvar_list = NULL;
int gvar_string_literal_label = 0;

void init_gvar_list() { gvar_list = new_vector(); }

GVar *add_gvar(GVar *gvar)
{
    vector_push_back(gvar_list, gvar);
    return gvar;
}

Vector *get_gvar_list() { return gvar_list; }

GVar *new_gvar_from_decl(AST *ast, int ival)
{
    assert(ast->kind == AST_GVAR_DECL);

    GVar *this;
    this = safe_malloc(sizeof(GVar));
    this->name = ast->varname;
    this->type = ast->type;
    this->ival = ival;
    this->sval = NULL;
    return this;
}

GVar *new_gvar_from_string_literal(char *sval, int ssize)
{
    GVar *this;
    this = safe_malloc(sizeof(GVar));
    this->name = format(".LC%d", gvar_string_literal_label++);
    this->type = new_array_type(type_char(), ssize);
    this->sval = sval;
    return this;
}

Vector *switch_cases = NULL;
char *default_label = NULL;

#define SAVE_SWITCH_CXT                                   \
    Vector *switch_cxt__prev_swtich_cases = switch_cases; \
    char *switch_cxt__default_label = default_label;      \
    switch_cases = new_vector();

#define RESTORE_SWITCH_CXT                        \
    switch_cases = switch_cxt__prev_swtich_cases; \
    default_label = switch_cxt__default_label;

Vector *get_switch_cases() { return switch_cases; }

char *get_default_label() { return default_label; }

Vector *reset_switch_cases(Vector *cases)
{
    Vector *prev = switch_cases;
    if (cases)
        switch_cases = cases;
    else
        switch_cases = new_vector();
    return prev;
}

AST *add_switch_case(AST *case_ast)
{
    // TODO: check duplicate cases
    AST *label = new_label_ast(make_label_string(), case_ast->rhs);

    SwitchCase *cas = (SwitchCase *)safe_malloc(sizeof(SwitchCase));
    cas->cond = case_ast->lhs->ival;
    cas->label_name = label->label_name;
    vector_push_back(switch_cases, cas);

    return label;
}

AST *add_switch_default(AST *default_ast)
{
    if (default_label)
        error("duplicate default label for switch", __FILE__, __LINE__);
    AST *label = new_label_ast(make_label_string(), default_ast->lhs);
    default_label = label->label_name;
    return label;
}

Vector *goto_asts;
Map *label_asts;

void init_goto_info()
{
    goto_asts = new_vector();
    label_asts = new_map();
}

AST *append_goto_ast(AST *ast)
{
    assert(ast->kind == AST_GOTO);
    vector_push_back(goto_asts, ast);
    return ast;
}

AST *append_label_ast(char *label_org_name, AST *ast)
{
    assert(ast->kind == AST_LABEL);
    map_insert(label_asts, label_org_name, ast);
    return ast;
}

void replace_goto_label()
{
    for (int i = 0; i < vector_size(goto_asts); i++) {
        AST *ast = (AST *)vector_get(goto_asts, i);
        KeyValue *kv = map_lookup(label_asts, ast->label_name);
        if (kv == NULL)
            error(format("not found such label: '%s'", kv_key(kv)), __FILE__,
                  __LINE__);
        ast->label_name = ((AST *)kv_value(kv))->label_name;
    }
}

void swap(AST **lhs, AST **rhs)
{
    AST *tmp;
    tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

int is_lvalue(AST *ast)
{
    if (ast == NULL) return 0;

    switch (ast->kind) {
        case AST_LVAR:
        case AST_GVAR:
        case AST_INDIR:
        case AST_MEMBER_REF:
            return 1;
    }

    return 0;
}

AST *lvalue2rvalue(AST *lvalue)
{
    if (is_lvalue(lvalue)) return new_lvalue2rvalue_ast(lvalue);
    return lvalue;
}

// not recursive.
Type *analyze_type(Env *env, Type *type)
{
    if (type->kind != TY_STRUCT) return type;
    if (type->members != NULL) return type;

    if (type->decls == NULL) {
        Type *ntype = lookup_type(env, type->stname);
        return ntype ? ntype : type;
    }

    type->members = new_vector();

    // make struct members.
    int size = vector_size(type->decls);
    for (int i = 0; i < size; i++) {
        // TODO: duplicate name
        AST *decl = (AST *)vector_get(type->decls, i);
        if (decl->kind != AST_STRUCT_VAR_DECL)
            error("struct should have only var decls", __FILE__, __LINE__);
        decl->type = analyze_type(env, decl->type);
        if (decl->varname == NULL &&
            (decl->type->kind != TY_STRUCT || decl->type->stname != NULL))
            continue;

        if (decl->type->kind == TY_STRUCT && decl->varname == NULL) {
            // nested anonymous struct with no varname.
            // its members can be accessed like parent struct's members.
            for (int i = 0; i < vector_size(decl->type->members); i++) {
                StructMember *member =
                    (StructMember *)vector_get(decl->type->members, i);
                vector_push_back(type->members, member);
            }
        }
        else {
            StructMember *member = safe_malloc(sizeof(StructMember));
            member->type = decl->type;
            member->name = decl->varname;
            vector_push_back(type->members, member);
        }
    }

    // calc offset
    size = vector_size(type->members);
    int offset = 0, alignment = alignment_of(type);
    for (int i = 0; i < size; i++) {
        StructMember *member = (StructMember *)vector_get(type->members, i);
        member->offset = offset;
        offset += roundup(member->type->nbytes, alignment);
    }
    type->nbytes = offset;

    if (type->stname) add_type(env, type);

    return type;
}

// Returns an analyzed AST pointer that may be the same one of `ast` or not.
// Caller should replace `ast` with the returned AST pointer.
AST *analyze_ast_detail(Env *env, AST *ast)
{
    if (ast == NULL) return NULL;

    switch (ast->kind) {
        case AST_INT:
            ast->type = type_int();
            break;

        case AST_STRING_LITERAL: {
            GVar *gvar =
                add_gvar(new_gvar_from_string_literal(ast->sval, ast->ssize));

            AST *ast;
            ast = new_ast(AST_GVAR);
            ast->varname = gvar->name;
            ast->type = gvar->type;
            return ast;
        } break;

        case AST_ADD:
            ast->lhs = char2int(
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->lhs))));
            ast->rhs = char2int(
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->rhs))));

            if (match_type2(ast->lhs, ast->rhs, TY_PTR, TY_PTR))
                error("ptr + ptr is not allowed", __FILE__, __LINE__);

            // Convert ptr+int to int+ptr for easy code generation.
            if (match_type(ast->lhs, TY_PTR)) swap(&ast->lhs, &ast->rhs);

            ast->type = ast->rhs->type;
            break;

        case AST_SUB:
            ast->lhs = char2int(
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->lhs))));
            ast->rhs = char2int(
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->rhs))));

            if (match_type2(ast->lhs, ast->rhs, TY_INT, TY_PTR))
                error("int - ptr is not allowed", __FILE__, __LINE__);

            if (match_type2(ast->lhs, ast->rhs, TY_PTR, TY_PTR))
                ast->type = type_int();  // TODO: long
            else
                ast->type = ast->lhs->type;
            break;

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
            // TODO: ensure both lhs and rhs have arithmetic types or pointer
            // types if needed.
            ast->lhs = char2int(
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->lhs))));
            ast->rhs = char2int(
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->rhs))));
            ast->type = ast->lhs->type;  // TODO: consider both lhs and rhs
            // TODO: ptr == ptr is okay, but ptr * ptr is not.
            break;

        case AST_NEQ:
            ast = new_unary_ast(AST_NOT,
                                new_binop_ast(AST_EQ, ast->lhs, ast->rhs));
            ast = analyze_ast_detail(env, ast);
            break;

        case AST_GTE:
        case AST_GT: {
            AST *lhs = char2int(
                    lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->lhs)))),
                *rhs = char2int(
                    lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->rhs))));
            // swap lhs and rhs to replace AST_GT(E) with AST_LT(E)
            ast->lhs = rhs;
            ast->rhs = lhs;
            ast->kind = ast->kind == AST_GT ? AST_LT : AST_LTE;
            ast->type = ast->lhs->type;  // TODO: consider both lhs and rhs
        } break;

        case AST_UNARY_MINUS:
            ast->lhs = char2int(
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->lhs))));
            ast->type = ast->lhs->type;
            if (ast->type->kind != TY_INT && ast->type->kind != TY_CHAR)
                error("expect int or char type for unary minus or not",
                      __FILE__, __LINE__);
            break;

        case AST_NOT:
            ast = new_binop_ast(AST_EQ, new_int_ast(0), ast->lhs);
            ast = analyze_ast_detail(env, ast);
            return ast;

        case AST_COND:
            ast->cond = char2int(
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->cond))));
            ast->then = char2int(
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->then))));
            ast->els = char2int(
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->els))));
            if (ast->then->type->kind != ast->els->type->kind)
                error("then and els should have the same type", __FILE__,
                      __LINE__);
            ast->type = ast->then->type;
            break;

        case AST_ASSIGN:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            ast->rhs =
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->rhs)));
            if (!is_lvalue(ast->lhs))
                error("should specify lvalue for assignment op", __FILE__,
                      __LINE__);
            ast->type = ast->lhs->type;
            break;

        case AST_VAR: {
            char *varname = ast->varname;

            ast = lookup_var(env, varname);
            if (!ast)
                error(format("not declared variable: '%s'", varname), __FILE__,
                      __LINE__);
            assert(ast->kind == AST_LVAR || ast->kind == AST_GVAR);
        } break;

        case AST_LVAR_DECL:
            ast->type = analyze_type(env, ast->type);
            if (ast->varname == NULL) return new_ast(AST_NOP);
            // ast->type means this variable's type and is alraedy
            // filled when parsing.
            assert(ast->type->nbytes > 0);
            add_var(env, ast);
            break;

        case AST_GVAR_DECL:
            ast->type = analyze_type(env, ast->type);
            if (ast->varname == NULL) return new_ast(AST_NOP);
            assert(ast->type->nbytes > 0);
            add_var(env, ast);
            add_gvar(new_gvar_from_decl(ast, 0));
            break;

        case AST_LVAR_DECL_INIT:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            ast->rhs = analyze_ast_detail(env, ast->rhs);
            break;

        case AST_GVAR_DECL_INIT:
            if (ast->lhs->kind == AST_GVAR_DECL) {
                if (ast->rhs->rhs->kind != AST_INT)
                    // TODO: constant, not int literal
                    error("global variable initializer must be constant.",
                          __FILE__, __LINE__);
                add_var(env, ast->lhs);
                add_gvar(new_gvar_from_decl(ast->lhs, ast->rhs->rhs->ival));
                ast->rhs = new_ast(AST_NOP);  // rhs should not be evaluated.
            }
            break;

        case AST_FUNCCALL: {
            AST *funcdef;
            int i;

            funcdef = lookup_func(env, ast->fname);
            if (funcdef) {  // found: already declared
                ast->type = funcdef->type;
            }
            else {
                warn("function call type is deduced to int", __FILE__,
                     __LINE__);
                warn(ast->fname, __FILE__, __LINE__);
                ast->type = type_int();
            }

            // analyze args
            for (i = 0; i < vector_size(ast->args); i++)
                vector_set(ast->args, i,
                           char2int(lvalue2rvalue(ary2ptr(analyze_ast_detail(
                               env, (AST *)vector_get(ast->args, i))))));
        } break;

        case AST_FUNC_DECL:
            add_func(env, ast->fname, ast);
            break;

        case AST_FUNCDEF: {
            AST *func;
            int i;

            func = lookup_func(env, ast->fname);
            if (func) {                  // already declared or defined.
                if (func->body != NULL)  // TODO: validate params
                    error(
                        "A function that has the same name as the defined "
                        "one "
                        "already exists.",
                        __FILE__, __LINE__);
                func->body = ast->body;
            }
            else {  // no declaration
                add_func(env, ast->fname, ast);
            }

            // make function's local scope/env
            ast->env = new_env(env);
            ast->env->scoped_vars = new_vector();
            // add param into functions's scope
            // in reversed order for easy code generation.
            for (i = vector_size(ast->params) - 1; i >= 0; i--)
                add_var(ast->env, (AST *)vector_get(ast->params, i));

            // analyze body
            init_goto_info();
            ast->body = analyze_ast_detail(ast->env, ast->body);
            replace_goto_label();
        } break;

        case AST_EXPR_STMT:
        case AST_RETURN: {
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            if (ast->lhs)  //
                ast->lhs = char2int(lvalue2rvalue(ary2ptr(ast->lhs)));
        } break;

        case AST_COMPOUND: {
            int i;
            Env *nenv;

            nenv = new_env(env);
            for (i = 0; i < vector_size(ast->stmts); i++)
                vector_set(
                    ast->stmts, i,
                    analyze_ast_detail(nenv, (AST *)vector_get(ast->stmts, i)));
        } break;

        case AST_IF:
            ast->cond = analyze_ast_detail(env, ast->cond);
            ast->then = analyze_ast_detail(env, ast->then);
            ast->els = analyze_ast_detail(env, ast->els);
            break;

        case AST_LABEL:
            ast->label_stmt = analyze_ast_detail(env, ast->label_stmt);
            append_label_ast(ast->label_name, ast);
            ast->label_name = make_label_string();
            break;

        case AST_CASE:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            ast->rhs = analyze_ast_detail(env, ast->rhs);
            if (ast->lhs->kind != AST_INT)
                error("case should take a constant expression.", __FILE__,
                      __LINE__);
            ast = add_switch_case(ast);
            break;

        case AST_DEFAULT:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            ast = add_switch_default(ast);
            break;

        case AST_SWITCH: {
            SAVE_SWITCH_CXT;
            ast->target = char2int(
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->target))));
            ast->switch_body = analyze_ast_detail(env, ast->switch_body);
            ast->cases = get_switch_cases();
            ast->default_label = get_default_label();
            RESTORE_SWITCH_CXT;
        } break;

        case AST_WHILE: {
            AST *cond = analyze_ast_detail(env, ast->cond),
                *body = analyze_ast_detail(env, ast->then);
            ast = new_ast(AST_FOR);
            ast->initer = NULL;
            ast->midcond = cond;
            ast->iterer = NULL;
            ast->for_body = body;
        } break;

        case AST_FOR:
            ast->initer = analyze_ast_detail(env, ast->initer);
            ast->midcond = analyze_ast_detail(env, ast->midcond);
            ast->iterer = analyze_ast_detail(env, ast->iterer);
            ast->for_body = analyze_ast_detail(env, ast->for_body);
            break;

        case AST_PREINC:
        case AST_POSTINC:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            if (!is_lvalue(ast->lhs))
                error("should specify lvalue for pre increment", __FILE__,
                      __LINE__);
            ast->type = ast->lhs->type;
            break;

        case AST_ADDR:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            if (!is_lvalue(ast->lhs))
                error("should specify lvalue for address-of op", __FILE__,
                      __LINE__);
            ast->type = new_pointer_type(ast->lhs->type);
            break;

        case AST_INDIR:  // = rvalue2lvalue
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            ast->lhs = lvalue2rvalue(ast->lhs);
            if (!match_type(ast->lhs, TY_PTR))
                error("pointer should come after indirection op", __FILE__,
                      __LINE__);
            ast->type = ast->lhs->type->ptr_of;
            break;

        case AST_ARY2PTR:
            assert(0);  // only made in analysis.

        case AST_GVAR:
            break;

        case AST_SIZEOF: {
            if (ast->lhs->kind == AST_NOP)
                ast->lhs->type = analyze_type(env, ast->lhs->type);
            else
                ast->lhs = analyze_ast_detail(env, ast->lhs);

            return new_int_ast(ast->lhs->type->nbytes);  // TODO: size_t
        } break;

        case AST_GOTO:
            append_goto_ast(ast);
            break;

        case AST_MEMBER_REF: {
            ast->stsrc = analyze_ast_detail(env, ast->stsrc);
            ast->stsrc->type = analyze_type(env, ast->stsrc->type);
            StructMember *sm =
                lookup_member(ast->stsrc->type->members, ast->member);
            if (sm == NULL) error("no such member", __FILE__, __LINE__);
            ast->type = sm->type;
        } break;

        case AST_MEMBER_REF_PTR:
            ast->kind = AST_MEMBER_REF;
            ast->stsrc = new_unary_ast(AST_INDIR, ast->stsrc);
            ast = analyze_ast_detail(env, ast);
            break;
    }

    return ast;
}

void analyze_ast(Vector *asts)
{
    int i;
    Env *env;

    env = new_env(NULL);
    init_gvar_list();

    for (i = 0; i < vector_size(asts); i++)
        vector_set(asts, i,
                   analyze_ast_detail(env, (AST *)vector_get(asts, i)));
}
