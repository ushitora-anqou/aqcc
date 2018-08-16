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

GVar *new_gvar_from_decl(AST *ast)
{
    assert(ast->kind == AST_GVAR_DECL);

    GVar *this;
    this = (GVar *)safe_malloc(sizeof(GVar));
    this->name = ast->varname;
    this->type = ast->type;
    this->ival = 0;
    this->sval = NULL;
    return this;
}

GVar *new_gvar_from_string_literal(char *sval, int ssize)
{
    GVar *this;
    this = (GVar *)safe_malloc(sizeof(GVar));
    this->name = format(".LC%d", gvar_string_literal_label++);
    this->type = new_array_type(type_char(), ssize);
    this->sval = sval;
    return this;
}

GVar *new_gvar_from_static_lvar(AST *lvar)
{
    assert(lvar->kind == AST_LVAR && lvar->type->is_static);

    char *gen_varname = make_label_string();
    lvar->gen_varname = gen_varname;

    GVar *this = (GVar *)safe_malloc(sizeof(GVar));
    this->name = gen_varname;
    this->type = lvar->type;
    this->ival = 0;
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
    if (default_label) error("duplicate default label for switch");
    AST *label = new_label_ast(make_label_string(), default_ast->lhs);
    default_label = label->label_name;
    return label;
}

Vector *va_start_params;

Vector *set_va_start_params(Vector *params) { va_start_params = params; }

int get_index_in_va_start_params(char *name)
{
    for (int i = 0; i < vector_size(va_start_params); i++) {
        AST *param = (AST *)vector_get(va_start_params, i);
        if (strcmp(param->varname, name) == 0) return i;
    }

    return -1;
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
        if (kv == NULL) error(format("not found such label: '%s'", kv_key(kv)));
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

int is_last_expr_in_list_lvalue(AST *ast)
{
    if (ast->kind != AST_EXPR_LIST) return 0;
    assert(vector_size(ast->exprs) >= 1);
    AST *last_expr = (AST *)vector_get(ast->exprs, vector_size(ast->exprs) - 1);
    return is_lvalue(last_expr) || is_last_expr_in_list_lvalue(last_expr);
}

AST *lvalue2rvalue(AST *lvalue)
{
    if (is_lvalue(lvalue)) return new_lvalue2rvalue_ast(lvalue);
    return lvalue;
}

int is_scalar_type(Type *type)
{
    switch (type->kind) {
        case TY_INT:
        case TY_CHAR:
        case TY_PTR:
            return 1;
    }
    return 0;
}

int is_complete_type(Type *type)
{
    switch (type->kind) {
        case TY_INT:
        case TY_CHAR:
        case TY_PTR:
            return 1;

        case TY_ARY:
            return is_complete_type(type->ary_of);

        case TY_STRUCT:
        case TY_UNION:
            return type->members != NULL;

        case TY_ENUM:
        case TY_TYPEDEF:
        case TY_VOID:
            return 0;
    }

    assert(0);
}

Type *lookup_member_type(Vector *members, char *member)
{
    // O(n)
    int size = vector_size(members);
    for (int i = 0; i < size; i++) {
        StructMember *sm = (StructMember *)vector_get(members, i);
        if ((sm->type->kind == TY_STRUCT || sm->type->kind == TY_UNION) &&
            sm->name == NULL) {
            // nested anonymous struct with no varname.
            // its members can be accessed like parent struct's members.
            Type *type = lookup_member_type(sm->type->members, member);
            if (type) return type;
        }
        else if (strcmp(sm->name, member) == 0) {
            return sm->type;
        }
    }
    return NULL;
}

Type *analyze_type(Env *env, Type *type)
{
    assert(type != NULL);

    int org_is_static = type->is_static;
    int org_is_extern = type->is_extern;

    switch (type->kind) {
        case TY_INT:
        case TY_CHAR:
            break;

        case TY_PTR:
            type = new_pointer_type(analyze_type(env, type->ptr_of));
            break;

        case TY_TYPEDEF:
            // typedef replacement
            type = analyze_type(env, lookup_type(env, type->typedef_name));
            break;

        case TY_ARY:
            type->ary_of = analyze_type(env, type->ary_of);
            if (!is_complete_type(type->ary_of))
                error("array consists of only complete typed elements");
            type = new_array_type(type->ary_of, type->len);
            break;

        case TY_UNION:
        case TY_STRUCT: {
            if (type->members != NULL) break;  // already analyzed

            if (type->decls == NULL) {  // only declaration e.g. struct A;
                Type *ntype =
                    lookup_struct_or_union_or_enum_type(env, type->stname);
                if (ntype != NULL && type->kind != ntype->kind)
                    error("struct/union specifier is wrong.");
                type = ntype ? ntype : type;
                break;
            }

            type->members = new_vector();

            // make struct/union members.
            int size = vector_size(type->decls);
            for (int i = 0; i < size; i++) {
                // TODO: duplicate name
                AST *decl = (AST *)vector_get(type->decls, i);
                if (decl->kind != AST_STRUCT_VAR_DECL)
                    error("struct/union should have only var decls");
                decl->type = analyze_type(env, decl->type);
                if (decl->varname == NULL && ((decl->type->kind != TY_STRUCT &&
                                               decl->type->kind != TY_UNION) ||
                                              decl->type->stname != NULL))
                    continue;

                StructMember *member = safe_malloc(sizeof(StructMember));
                member->type = decl->type;
                member->name = decl->varname;
                vector_push_back(type->members, member);
            }

            if (type->kind == TY_STRUCT) {
                // calc offset
                size = vector_size(type->members);
                int offset = 0;
                for (int i = 0; i < size; i++) {
                    StructMember *member =
                        (StructMember *)vector_get(type->members, i);
                    offset = roundup(offset, alignment_of(member->type));
                    member->offset = offset;
                    offset += member->type->nbytes;
                }
                type->nbytes = roundup(offset, alignment_of(type));
            }
            else {  // if union
                // offset is always zero.
                int max_nbytes = 0;
                for (int i = 0; i < vector_size(type->members); i++) {
                    StructMember *member =
                        (StructMember *)vector_get(type->members, i);
                    member->offset = 0;
                    max_nbytes = max(max_nbytes, member->type->nbytes);
                }
                type->nbytes = roundup(max_nbytes, alignment_of(type));
            }

            if (type->stname) add_struct_or_union_or_enum_type(env, type);
        } break;

        case TY_ENUM: {
            if (type->enum_list == NULL) {  // only declaration e.g. enum A;
                Type *ntype =
                    lookup_struct_or_union_or_enum_type(env, type->enname);
                if (ntype == NULL) break;
                if (ntype->kind != TY_ENUM) error("enum specifier is wrong.");
                type = type_int();  // TODO: assume int
                break;
            }

            for (int i = 0, cnt = 0; i < vector_size(type->enum_list);
                 i++, cnt++) {
                AST *ast = (AST *)vector_get(type->enum_list, i);

                switch (ast->kind) {
                    case AST_ENUM_VAR_DECL:
                        add_enum_value(env, ast->varname, cnt);
                        break;

                    case AST_ENUM_VAR_DECL_INIT:
                        // TODO: constant expr is not only int literal
                        if (ast->rhs->rhs->kind != AST_INT)
                            error(
                                "constant expression is needed for enum "
                                "initializer");
                        add_enum_value(env, ast->lhs->varname,
                                       cnt = ast->rhs->rhs->ival);
                        break;

                    default:
                        assert(0);
                }
            }

            if (type->enname) add_struct_or_union_or_enum_type(env, type);
        } break;
    }

    if (org_is_static) type = new_static_type(type);
    if (org_is_extern) type = new_extern_type(type);
    return type;
}

AST *convert_expr(AST *expr)
{
    if (expr == NULL || expr->type == NULL) return expr;  // stmt
    // expr shouldn't have void type when convertible.
    // TODO: this should be an error.
    assert(expr->type->kind != TY_VOID);
    return char2int(lvalue2rvalue(ary2ptr(expr)));
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
            ast = new_lgvar_ast(AST_GVAR, gvar->type, gvar->name, -1);
        } break;

        case AST_ADD:
            ast->lhs = convert_expr(analyze_ast_detail(env, ast->lhs));
            ast->rhs = convert_expr(analyze_ast_detail(env, ast->rhs));

            if (match_type2(ast->lhs, ast->rhs, TY_PTR, TY_PTR))
                error("ptr + ptr is not allowed");

            // Convert ptr+int to int+ptr for easy code generation.
            if (match_type(ast->lhs, TY_PTR)) swap(&ast->lhs, &ast->rhs);

            ast->type = ast->rhs->type;
            break;

        case AST_SUB:
            ast->lhs = convert_expr(analyze_ast_detail(env, ast->lhs));
            ast->rhs = convert_expr(analyze_ast_detail(env, ast->rhs));

            if (match_type2(ast->lhs, ast->rhs, TY_INT, TY_PTR))
                error("int - ptr is not allowed");

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
            // TODO: ensure both lhs and rhs have arithmetic types or
            // pointer types if needed.
            ast->lhs = convert_expr(analyze_ast_detail(env, ast->lhs));
            ast->rhs = convert_expr(analyze_ast_detail(env, ast->rhs));
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
            AST *lhs = convert_expr(analyze_ast_detail(env, ast->lhs)),
                *rhs = convert_expr(analyze_ast_detail(env, ast->rhs));
            // swap lhs and rhs to replace AST_GT(E) with AST_LT(E)
            ast->lhs = rhs;
            ast->rhs = lhs;
            ast->kind = ast->kind == AST_GT ? AST_LT : AST_LTE;
            ast->type = ast->lhs->type;  // TODO: consider both lhs and rhs
        } break;

        case AST_COMPL:
        case AST_UNARY_MINUS:
            ast->lhs = convert_expr(analyze_ast_detail(env, ast->lhs));
            ast->type = ast->lhs->type;
            if (ast->type->kind != TY_INT && ast->type->kind != TY_CHAR)
                error("expect int or char type for unary minus or compl");
            break;

        case AST_NOT:
            ast = new_binop_ast(AST_EQ, new_int_ast(0), ast->lhs);
            ast = analyze_ast_detail(env, ast);
            return ast;

        case AST_COND:
            ast->cond = convert_expr(analyze_ast_detail(env, ast->cond));
            ast->then = convert_expr(analyze_ast_detail(env, ast->then));
            ast->els = convert_expr(analyze_ast_detail(env, ast->els));
            if (ast->then->type->kind != ast->els->type->kind)
                error("then and els should have the same type");
            ast->type = ast->then->type;
            break;

        case AST_ASSIGN:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            ast->rhs =
                lvalue2rvalue(ary2ptr(analyze_ast_detail(env, ast->rhs)));
            if (!is_lvalue(ast->lhs))
                error("should specify lvalue for assignment op");
            ast->type = ast->lhs->type;
            break;

        case AST_EXPR_LIST:
            for (int i = 0; i < vector_size(ast->exprs); i++) {
                AST *expr = (AST *)vector_get(ast->exprs, i);
                expr = analyze_ast_detail(env, expr);
                ast->type = expr->type;
                vector_set(ast->exprs, i, expr);
            }
            break;

        case AST_VAR: {
            char *varname = ast->varname;

            ast = lookup_var(env, varname);
            if (!ast) {
                // maybe enum
                int *ival = lookup_enum_value(env, varname);
                if (!ival)
                    error(format("not declared variable: '%s'", varname));
                ast = analyze_ast_detail(env, new_int_ast(*ival));
            }
        } break;

        case AST_LVAR_DECL: {
            // ast->type means this variable's type and is alraedy
            // filled when parsing.
            ast->type = analyze_type(env, ast->type);
            if (ast->varname == NULL) {
                ast = new_ast(AST_NOP);
                break;
            }
            if (!is_complete_type(ast->type))
                error("incomplete typed variable can't be declared.");
            assert(ast->type->nbytes > 0);
            AST *var = add_var(env, ast);

            if (var->type->is_static) add_gvar(new_gvar_from_static_lvar(var));
        } break;

        case AST_GVAR_DECL:
            // ast->type means this variable's type and is alraedy
            // filled when parsing.
            ast->type = analyze_type(env, ast->type);
            if (ast->varname == NULL) {
                ast = new_ast(AST_NOP);
                break;
            }
            if (!is_complete_type(ast->type))
                error("incomplete typed variable can't be declared.");
            assert(ast->type->nbytes > 0);
            add_var(env, ast);
            if (!ast->type->is_extern) add_gvar(new_gvar_from_decl(ast));
            break;

        case AST_TYPEDEF_VAR_DECL:
            ast->type = analyze_type(env, ast->type);
            add_type(env, ast->type, ast->varname);
            ast = new_ast(AST_NOP);
            break;

        case AST_LVAR_DECL_INIT:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            if (ast->lhs->type->is_extern)
                error("extern variable can't have any initializer: '%s'",
                      ast->lhs->varname);
            if (ast->lhs->type->is_static) {
                if (ast->rhs->rhs->kind != AST_INT)
                    // TODO: constant, not int literal
                    error(
                        "static local variable initializer must be constant.");
                Vector *gvar_list = get_gvar_list();
                GVar *gvar =
                    (GVar *)vector_get(gvar_list, vector_size(gvar_list) - 1);
                gvar->ival = ast->rhs->rhs->ival;
                ast->rhs = new_ast(AST_NOP);  // rhs should not be evaluated.
            }
            else {
                ast->rhs = analyze_ast_detail(env, ast->rhs);
            }
            break;

        case AST_GVAR_DECL_INIT: {
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            if (ast->rhs->rhs->kind != AST_INT)
                // TODO: constant, not int literal
                error("global variable initializer must be constant.");
            if (ast->lhs->type->is_extern)
                error("extern variable can't have any initializer: '%s'",
                      ast->lhs->varname);
            Vector *gvar_list = get_gvar_list();
            GVar *gvar =
                (GVar *)vector_get(gvar_list, vector_size(gvar_list) - 1);
            gvar->ival = ast->rhs->rhs->ival;
            ast->rhs = new_ast(AST_NOP);  // rhs should not be evaluated.
        } break;

        case AST_DECL_LIST:
            for (int i = 0; i < vector_size(ast->decls); i++) {
                AST *decl = (AST *)vector_get(ast->decls, i);
                decl = analyze_ast_detail(env, decl);
                vector_set(ast->decls, i, decl);
            }
            break;

        case AST_FUNCCALL: {
            AST *funcdef = lookup_func(env, ast->fname);
            if (funcdef) {  // found: already declared
                ast->type = analyze_type(env, funcdef->type);
            }
            else {
                warn("function \"%s\" call type is deduced to int", ast->fname);
                ast->type = type_int();
            }

            // analyze args
            for (int i = 0; i < vector_size(ast->args); i++)
                vector_set(ast->args, i,
                           convert_expr(analyze_ast_detail(
                               env, (AST *)vector_get(ast->args, i))));
        } break;

        case AST_FUNC_DECL:
            add_func(env, ast->fname, ast);
            break;

        case AST_FUNCDEF: {
            AST *func = lookup_func(env, ast->fname);
            if (func) {                  // already declared or defined.
                if (func->body != NULL)  // TODO: validate params
                    error(
                        "A function that has the same name as the defined "
                        "one already exists.");
                func->body = ast->body;
            }
            else {  // no declaration
                add_func(env, ast->fname, ast);
            }
            ast->type = analyze_type(env, ast->type);

            // make function's local scope/env
            ast->env = new_env(env);
            ast->env->scoped_vars = new_vector();
            // add param into functions's scope
            // in reversed order for easy code generation.
            if (ast->params) {
                for (int i = vector_size(ast->params) - 1; i >= 0; i--) {
                    AST *param = (AST *)vector_get(ast->params, i);
                    param->type = analyze_type(env, param->type);
                    if (param->type->kind == TY_ARY)
                        param->type = new_pointer_type(param->type->ary_of);
                    add_var(ast->env, param);
                }
            }

            // analyze body
            init_goto_info();
            set_va_start_params(ast->params);
            ast->body = analyze_ast_detail(ast->env, ast->body);
            replace_goto_label();
        } break;

        case AST_EXPR_STMT:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            break;

        case AST_RETURN:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            if (ast->lhs) ast->lhs = convert_expr(ast->lhs);
            break;

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
            ast->cond = convert_expr(analyze_ast_detail(env, ast->cond));
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
                error("case should take a constant expression.");
            ast = add_switch_case(ast);
            break;

        case AST_DEFAULT:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            ast = add_switch_default(ast);
            break;

        case AST_SWITCH: {
            SAVE_SWITCH_CXT;
            ast->target = convert_expr(analyze_ast_detail(env, ast->target));
            ast->switch_body = analyze_ast_detail(env, ast->switch_body);
            ast->cases = get_switch_cases();
            ast->default_label = get_default_label();
            RESTORE_SWITCH_CXT;
        } break;

        case AST_WHILE: {
            AST *cond = ast->cond, *body = ast->then;
            ast = new_ast(AST_FOR);
            ast->initer = NULL;
            ast->midcond = cond;
            ast->iterer = NULL;
            ast->for_body = body;
            ast = analyze_ast_detail(env, ast);
        } break;

        case AST_DOWHILE: {
            ast->cond = convert_expr(analyze_ast_detail(env, ast->cond));
            ast->then = analyze_ast_detail(env, ast->then);
        } break;

        case AST_FOR: {
            Env *nenv = new_env(env);
            ast->initer = convert_expr(analyze_ast_detail(nenv, ast->initer));
            ast->midcond = convert_expr(analyze_ast_detail(nenv, ast->midcond));
            ast->iterer = convert_expr(analyze_ast_detail(nenv, ast->iterer));
            ast->for_body = analyze_ast_detail(nenv, ast->for_body);
        } break;

        case AST_PREINC:
        case AST_POSTINC:
        case AST_PREDEC:
        case AST_POSTDEC:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            if (!is_lvalue(ast->lhs))
                error("should specify lvalue for increment/decrement");
            ast->type = ast->lhs->type;
            break;

        case AST_ADDR:
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            if (!is_lvalue(ast->lhs))
                error("should specify lvalue for address-of op");
            ast->type = new_pointer_type(ast->lhs->type);
            break;

        case AST_INDIR:  // = rvalue2lvalue
            ast->lhs = analyze_ast_detail(env, ast->lhs);
            ast->lhs = lvalue2rvalue(ast->lhs);
            if (!match_type(ast->lhs, TY_PTR))
                error("pointer should come after indirection op");
            ast->type = analyze_type(env, ast->lhs->type->ptr_of);
            if (!is_complete_type(ast->type))
                error("indirection op needs complete type");
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
            if (!is_lvalue(ast->stsrc) &&
                !is_last_expr_in_list_lvalue(ast->stsrc))
                error("lhs of dot should be lvalue");
            ast->stsrc->type = analyze_type(env, ast->stsrc->type);
            if (ast->stsrc->type->kind != TY_STRUCT &&
                ast->stsrc->type->kind != TY_UNION)
                error("only struct/union can have members");
            if (!is_complete_type(ast->stsrc->type))
                error("can't access imcomplete typed struct members");
            Type *type =
                lookup_member_type(ast->stsrc->type->members, ast->member);
            if (type == NULL) error("no such member");
            ast->type = type;
        } break;

        case AST_MEMBER_REF_PTR:
            ast->kind = AST_MEMBER_REF;
            ast->stsrc = new_unary_ast(AST_INDIR, ast->stsrc);
            ast = analyze_ast_detail(env, ast);
            break;

        case AST_CAST:
            if (!is_scalar_type(ast->type))
                error("only scalar type can be cast");
            // TODO: always convert_expr; is that safe?
            ast->lhs = convert_expr(analyze_ast_detail(env, ast->lhs));
            ast->lhs->type = ast->type;
            ast = ast->lhs;
            break;

        case AST_VA_START: {
            if (vector_size(ast->args) != 2)
                error("invalid number of arguments of va_start()");
            AST *lhs = (AST *)vector_get(ast->args, 0),
                *rhs = (AST *)vector_get(ast->args, 1);
            lhs = analyze_ast_detail(env, lhs);
            rhs = analyze_ast_detail(env, rhs);
            if (rhs->kind != AST_LVAR) error("invalid argument of va_start()");
            int index = get_index_in_va_start_params(rhs->varname);
            if (index < 0)
                error("invalid argument of va_start(): not such param");
            ast = new_binop_ast(AST_VA_START, lhs, new_int_ast(index + 1));
            ast->type = type_void();
        } break;

        case AST_VA_END: {
            ast = new_ast(AST_NOP);
        } break;
    }

    return ast;
}

void analyze_ast(Vector *asts)
{
    Env *env = new_env(NULL);
    init_gvar_list();

    for (int i = 0; i < vector_size(asts); i++)
        vector_set(asts, i,
                   analyze_ast_detail(env, (AST *)vector_get(asts, i)));
}
