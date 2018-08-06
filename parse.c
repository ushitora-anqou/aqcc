#include "aqcc.h"

AST *parse_expr();
AST *parse_assignment_expr();
AST *parse_stmt();
Type *parse_type_name();
AST *parse_var_declaration(int kind);

_Noreturn void error_unexpected_token_kind(int expect_kind, Token *got)
{
    error(format(":%d:%d: unexpected token: expect %s, got %s", got->line,
                 got->column, token_kind2str(expect_kind),
                 token_kind2str(got->kind)),
          __FILE__, __LINE__);
}

_Noreturn void error_unexpected_token_str(char *expect_str, Token *got)
{
    error(format(":%d:%d: unexpected token: expect %s, got %s", got->line,
                 got->column, expect_str, token_kind2str(got->kind)),
          __FILE__, __LINE__);
}

TokenSeq *new_token_seq(Vector *tokens)
{
    TokenSeq *this = safe_malloc(sizeof(TokenSeq));

    this->tokens = tokens;
    this->idx = 0;
    return this;
}

TokenSeq *tokenseq;

void init_tokenseq(Vector *tokens) { tokenseq = new_token_seq(tokens); }

Token *peek_token()
{
    Token *token = vector_get(tokenseq->tokens, tokenseq->idx);
    if (token == NULL) error("no next token.", __FILE__, __LINE__);
    return token;
}

Token *pop_token()
{
    Token *token = vector_get(tokenseq->tokens, tokenseq->idx++);
    if (token == NULL) error("no next token.", __FILE__, __LINE__);
    return token;
}

Token *expect_token(int kind)
{
    Token *token = pop_token(tokenseq);
    if (token->kind != kind) error_unexpected_token_kind(kind, token);
    return token;
}

int match_token(int kind)
{
    Token *token = peek_token(tokenseq);
    return token->kind == kind;
}

Token *pop_token_if(int kind)
{
    if (match_token(kind)) return pop_token();
    return NULL;
}

int match_token2(int kind0, int kind1)
{
    Token *token = peek_token(tokenseq);
    if (token->kind != kind0) return 0;
    tokenseq->idx++;
    token = peek_token(tokenseq);
    tokenseq->idx--;
    if (token->kind != kind1) return 0;
    return 1;
}

int match_type_specifier()
{
    int kind = peek_token()->kind;
    return kind == kINT || kind == kCHAR || kind == kSTRUCT;
}

TokenSeqSaved *new_token_seq_saved()
{
    TokenSeqSaved *this;

    this = (TokenSeqSaved *)safe_malloc(sizeof(TokenSeqSaved));
    this->idx = tokenseq->idx;

    return this;
}

void restore_token_seq_saved(TokenSeqSaved *saved)
{
    tokenseq->idx = saved->idx;
}

#define SAVE_TOKENSEQ \
    TokenSeqSaved *token_seq_saved__dummy = new_token_seq_saved();
#define RESTORE_TOKENSEQ restore_token_seq_saved(token_seq_saved__dummy);

AST *parse_primary_expr()
{
    AST *ast;
    Token *token = pop_token();
    switch (token->kind) {
        case tINT:
            ast = new_ast(AST_INT);
            ast->ival = token->ival;
            break;

        case tLPAREN:
            ast = parse_expr();
            expect_token(tRPAREN);
            break;

        case tSTRING_LITERAL:
            ast = new_ast(AST_STRING_LITERAL);
            ast->sval = token->sval;
            ast->ssize = token->ssize;
            break;

        case tIDENT:
            // ALL tIDENT should be handled in parse_postfix_expr()
            assert(0);

        default:
            error_unexpected_token_str("primary expression", token);
    }
    return ast;
}

Vector *parse_argument_expr_list()
{
    Vector *args = new_vector();

    if (match_token(tRPAREN)) return args;

    while (1) {
        vector_push_back(args, parse_assignment_expr());
        if (match_token(tRPAREN)) break;
        expect_token(tCOMMA);
    }

    return args;
}

AST *parse_postfix_expr()
{
    // TODO: should be recursive

    if (match_token2(tIDENT, tLPAREN)) {  // function call
        Token *ident;
        AST *ast;

        ident = pop_token();
        pop_token();

        ast = new_func_ast(AST_FUNCCALL, ident->sval,
                           parse_argument_expr_list(), NULL, NULL);
        expect_token(tRPAREN);
        return ast;
    }

    AST *ast;

    if (match_token(tIDENT))  // variable
        ast = new_var_ast(pop_token()->sval);
    else  // primary expr
        ast = parse_primary_expr();

    while (1) {
        switch (peek_token()->kind) {
            case tINC: {
                pop_token();
                ast = new_unary_ast(AST_POSTINC, ast);
            } break;

            case tLBRACKET:
                pop_token();
                ast = new_unary_ast(AST_INDIR,
                                    new_binop_ast(AST_ADD, parse_expr(), ast));
                expect_token(tRBRACKET);
                break;

            case tDOT: {
                pop_token();
                char *ident = expect_token(tIDENT)->sval;
                AST *nast = new_ast(AST_MEMBER_REF);
                nast->stsrc = ast;
                nast->member = ident;
                ast = nast;
            } break;

            case tARROW: {
                pop_token();
                char *ident = expect_token(tIDENT)->sval;
                AST *nast = new_ast(AST_MEMBER_REF_PTR);
                nast->stsrc = ast;
                nast->member = ident;
                ast = nast;
            } break;

            default:
                goto end;
        }
    }

end:
    return ast;
}

AST *parse_unary_expr()
{
    Token *token = peek_token();
    switch (token->kind) {
        case tPLUS:
            pop_token();
            return parse_postfix_expr();

        case tMINUS:
            pop_token();
            return new_unary_ast(AST_UNARY_MINUS, parse_unary_expr());

        case tINC:
            pop_token();
            return new_unary_ast(AST_PREINC, parse_unary_expr());

        case tAND:
            pop_token();
            return new_unary_ast(AST_ADDR, parse_unary_expr());

        case tSTAR:
            pop_token();
            return new_unary_ast(AST_INDIR, parse_unary_expr());

        case kSIZEOF: {
            AST *ast;
            ast = new_ast(AST_SIZEOF);

            pop_token();
            expect_token(tLPAREN);  // TODO: parse no lparen sizeof
            if (match_type_specifier()) {
                ast->lhs = new_ast(AST_NOP);
                ast->lhs->type = parse_type_name();
            }
            else {
                ast->lhs = parse_unary_expr();
            }
            expect_token(tRPAREN);

            return ast;
        } break;
    }

    return parse_postfix_expr();
}

AST *parse_multiplicative_expr()
{
    AST *ast = parse_unary_expr();

    while (1) {
        Token *token = peek_token();
        switch (token->kind) {
            case tSTAR:
                pop_token();
                ast = new_binop_ast(AST_MUL, ast, parse_unary_expr());
                break;

            case tSLASH:
                pop_token();
                ast = new_binop_ast(AST_DIV, ast, parse_unary_expr());
                break;

            case tPERCENT:
                pop_token();
                ast = new_binop_ast(AST_REM, ast, parse_unary_expr());
                break;

            default:
                return ast;
        }
    }
}

AST *parse_additive_expr()
{
    AST *lhs, *rhs;

    lhs = parse_multiplicative_expr();

    while (1) {
        Token *token = peek_token();
        switch (token->kind) {
            case tPLUS:

                pop_token();
                rhs = parse_multiplicative_expr();
                lhs = new_binop_ast(AST_ADD, lhs, rhs);
                break;

            case tMINUS:
                pop_token();
                rhs = parse_multiplicative_expr();
                lhs = new_binop_ast(AST_SUB, lhs, rhs);
                break;

            default:
                return lhs;
        }
    }
}

AST *parse_shift_expr()
{
    AST *ast = parse_additive_expr();

    while (1) {
        Token *token = peek_token();
        switch (token->kind) {
            case tLSHIFT:
                pop_token();
                ast = new_binop_ast(AST_LSHIFT, ast, parse_additive_expr());
                break;
            case tRSHIFT:
                pop_token();
                ast = new_binop_ast(AST_RSHIFT, ast, parse_additive_expr());
                break;
            default:
                return ast;
        }
    }
}

AST *parse_relational_expr()
{
    AST *ast = parse_shift_expr();

    while (1) {
        Token *token = peek_token();
        switch (token->kind) {
            case tLT:
                pop_token();
                ast = new_binop_ast(AST_LT, ast, parse_shift_expr());
                break;

            case tGT:
                pop_token();
                ast = new_binop_ast(AST_GT, ast, parse_shift_expr());
                break;

            case tLTE:
                pop_token();
                ast = new_binop_ast(AST_LTE, ast, parse_shift_expr());
                break;

            case tGTE:
                pop_token();
                ast = new_binop_ast(AST_GTE, ast, parse_shift_expr());
                break;

            default:
                return ast;
        }
    }
}

AST *parse_equality_expr()
{
    AST *ast = parse_relational_expr();

    while (1) {
        Token *token = peek_token();
        switch (token->kind) {
            case tEQEQ:
                pop_token();
                ast = new_binop_ast(AST_EQ, ast, parse_relational_expr());
                break;

            case tNEQ:
                pop_token();
                ast = new_binop_ast(AST_NEQ, ast, parse_relational_expr());
                break;

            default:
                return ast;
        }
    }
}

AST *parse_and_expr()
{
    AST *ast = parse_equality_expr();

    while (1) {
        if (peek_token()->kind != tAND) return ast;
        pop_token();
        ast = new_binop_ast(AST_AND, ast, parse_equality_expr());
    }
}

AST *parse_exclusive_or_expr()
{
    AST *ast = parse_and_expr();

    while (1) {
        if (peek_token()->kind != tHAT) return ast;
        pop_token();
        ast = new_binop_ast(AST_XOR, ast, parse_and_expr());
    }
}

AST *parse_inclusive_or_expr()
{
    AST *ast = parse_exclusive_or_expr();

    while (1) {
        if (peek_token()->kind != tBAR) return ast;
        pop_token();
        ast = new_binop_ast(AST_OR, ast, parse_exclusive_or_expr());
    }
}

AST *parse_logical_and_expr()
{
    AST *ast = parse_inclusive_or_expr();

    while (1) {
        if (peek_token()->kind != tANDAND) return ast;
        pop_token();
        ast = new_binop_ast(AST_LAND, ast, parse_inclusive_or_expr());
    }
}

AST *parse_logical_or_expr()
{
    AST *ast = parse_logical_and_expr();

    while (1) {
        if (peek_token()->kind != tBARBAR) return ast;
        pop_token();
        ast = new_binop_ast(AST_LOR, ast, parse_logical_and_expr());
    }
}

AST *parse_conditional_expr()
{
    AST *cond, *then, *els, *ast;

    cond = parse_logical_or_expr();
    if (!match_token(tQUESTION)) return cond;
    pop_token();
    then = parse_expr();
    expect_token(tCOLON);
    els = parse_conditional_expr();

    ast = new_ast(AST_COND);
    ast->cond = cond;
    ast->then = then;
    ast->els = els;
    return ast;
}

AST *parse_assignment_expr()
{
    static int tok2ast[256];  // TODO: number of ast kind;  enough?
    AST *last, *rast;
    Token *token;
    int kind;

    if (tok2ast[0] == 0) {                       // if not initialized
        memset(tok2ast, 0xff, sizeof(tok2ast));  // fill tok2ast with -1
        tok2ast[tEQ] = AST_NOP;
        tok2ast[tPLUSEQ] = AST_ADD;
        tok2ast[tMINUSEQ] = AST_SUB;
        tok2ast[tSTAREQ] = AST_MUL;
        tok2ast[tSLASHEQ] = AST_DIV;
        tok2ast[tPERCENTEQ] = AST_REM;
        tok2ast[tANDEQ] = AST_AND;
        tok2ast[tHATEQ] = AST_XOR;
        tok2ast[tBAREQ] = AST_OR;
        tok2ast[tLSHIFTEQ] = AST_LSHIFT;
        tok2ast[tRSHIFTEQ] = AST_RSHIFT;
    }

    SAVE_TOKENSEQ;
    last = parse_unary_expr();
    token = pop_token();
    kind = tok2ast[token->kind];
    if (kind == -1) {
        RESTORE_TOKENSEQ;
        return parse_conditional_expr();
    }

    rast = parse_assignment_expr();
    if (kind != AST_NOP) rast = new_binop_ast(kind, last, rast);
    return new_binop_ast(AST_ASSIGN, last, rast);
}

AST *parse_expr() { return parse_assignment_expr(); }

AST *parse_expression_stmt()
{
    AST *expr = NULL;

    if (!match_token(tSEMICOLON)) expr = parse_expr();
    expect_token(tSEMICOLON);

    return new_unary_ast(AST_EXPR_STMT, expr);
}

AST *parse_jump_stmt()
{
    Token *token = pop_token();
    AST *ast = NULL;

    switch (token->kind) {
        case kRETURN:
            if (!match_token(tSEMICOLON)) ast = parse_expr();
            expect_token(tSEMICOLON);
            ast = new_unary_ast(AST_RETURN, ast);
            break;

        case kBREAK:
            expect_token(tSEMICOLON);
            ast = new_ast(AST_BREAK);
            break;

        case kCONTINUE:
            expect_token(tSEMICOLON);
            ast = new_ast(AST_CONTINUE);
            break;

        case kGOTO:
            token = expect_token(tIDENT);
            expect_token(tSEMICOLON);
            ast = new_ast(AST_GOTO);
            ast->label_name = token->sval;
            break;

        default:
            error_unexpected_token_str("jump statement", token);
    }

    return ast;
}

AST *parse_iteration_stmt()
{
    Token *token = pop_token();
    AST *ast;

    switch (token->kind) {
        case kWHILE: {
            AST *cond, *body;
            expect_token(tLPAREN);
            cond = parse_expr();
            expect_token(tRPAREN);
            body = parse_stmt();

            ast = new_while_stmt(cond, body);
        } break;

        case kFOR: {
            AST *initer, *cond, *iterer, *body;

            expect_token(tLPAREN);
            if (match_token(tSEMICOLON))
                initer = NULL;
            else
                initer = parse_expr();
            expect_token(tSEMICOLON);
            if (match_token(tSEMICOLON))
                cond = NULL;
            else
                cond = parse_expr();
            expect_token(tSEMICOLON);
            if (match_token(tRPAREN))
                iterer = NULL;
            else
                iterer = parse_expr();
            expect_token(tRPAREN);
            body = parse_stmt();

            ast = new_ast(AST_FOR);
            ast->initer = initer;
            ast->midcond = cond;
            ast->iterer = iterer;
            ast->for_body = body;
        } break;

        default:
            error_unexpected_token_str("iteraiton statement", token);
    }

    return ast;
}

Type *parse_struct_specifier()
{
    expect_token(kSTRUCT);

    char *name = NULL;
    if (match_token(tIDENT)) name = pop_token()->sval;

    if (!match_token(tLBRACE)) {
        if (name == NULL)
            error("struct specifier should have a tIDENT or declaration list",
                  __FILE__, __LINE__);
        return new_struct_type(name, NULL);
    }
    pop_token();

    Vector *decls = new_vector();
    while (!match_token(tRBRACE)) {
        vector_push_back(decls, parse_var_declaration(AST_STRUCT_VAR_DECL));
        expect_token(tSEMICOLON);
    }
    pop_token();

    return new_struct_type(name, decls);
}

Type *parse_type_specifier()
{
    Token *token;

    token = peek_token();
    switch (token->kind) {
        case kINT:
            pop_token();
            return type_int();
        case kCHAR:
            pop_token();
            return type_char();
        case kSTRUCT:
            return parse_struct_specifier();
    }
    error_unexpected_token_str("type specifier", token);
}

Type *parse_type_name()
{
    Type *type;

    type = parse_type_specifier();

    while (match_token(tSTAR)) {
        pop_token();
        type = new_pointer_type(type);
    }

    return type;
}

AST *parse_var_declaration(int kind)
{
    Type *type = parse_type_name();
    char *varname = NULL;
    if (match_token(tIDENT)) varname = pop_token()->sval;
    AST *ast = new_var_decl_ast(kind, type, varname);

    if (varname == NULL) return ast;

    while (match_token(tLBRACKET)) {  // array
        Token *num;

        pop_token();
        num = expect_token(tINT);  // TODO: parse assignment-expr
        expect_token(tRBRACKET);
        ast->type = new_array_type(ast->type, num->ival);
    }

    return ast;
}

AST *parse_var_init_declaration(int kind)
{
    AST *ast = parse_var_declaration(kind);

    if (match_token(tEQ)) {  // initializer
        pop_token();
        ast = new_var_decl_init_ast(ast, parse_assignment_expr());
    }

    expect_token(tSEMICOLON);

    return ast;
}

AST *parse_compound_stmt()
{
    AST *ast;
    Vector *stmts = new_vector();

    expect_token(tLBRACE);
    while (!match_token(tRBRACE)) {
        if (match_type_specifier())
            vector_push_back(stmts, parse_var_init_declaration(AST_LVAR_DECL));
        else
            vector_push_back(stmts, parse_stmt());
    }
    expect_token(tRBRACE);

    ast = new_ast(AST_COMPOUND);
    ast->stmts = stmts;

    return ast;
}

AST *parse_selection_stmt()
{
    if (match_token(kIF)) {
        pop_token();
        expect_token(tLPAREN);
        AST *cond = parse_expr();
        expect_token(tRPAREN);
        AST *then = parse_stmt();
        AST *els = NULL;
        if (match_token(kELSE)) {
            pop_token();
            els = parse_stmt();
        }

        AST *ast = new_ast(AST_IF);
        ast->cond = cond;
        ast->then = then;
        ast->els = els;
        return ast;
    }
    else if (match_token(kSWITCH)) {
        pop_token();
        expect_token(tLPAREN);
        AST *cond = parse_expr();
        expect_token(tRPAREN);
        AST *body = parse_stmt();

        AST *ast = new_ast(AST_SWITCH);
        ast->target = cond;
        ast->switch_body = body;
        return ast;
    }

    error_unexpected_token_str("selection statement", pop_token());
}

AST *parse_constant_expr() { return parse_conditional_expr(); }

AST *parse_labeled_stmt()
{
    if (match_token(kCASE)) {
        pop_token();
        AST *cnst = parse_constant_expr();
        expect_token(tCOLON);
        AST *stmt = parse_stmt();

        return new_binop_ast(AST_CASE, cnst, stmt);
    }
    else if (match_token(kDEFAULT)) {
        pop_token();
        expect_token(tCOLON);
        AST *stmt = parse_stmt();
        return new_unary_ast(AST_DEFAULT, stmt);
    }

    char *label_name = expect_token(tIDENT)->sval;
    expect_token(tCOLON);
    AST *stmt = parse_stmt();
    return new_label_ast(label_name, stmt);
}

AST *parse_stmt()
{
    Token *token = peek_token();

    switch (token->kind) {
        case kBREAK:
        case kCONTINUE:
        case kRETURN:
        case kGOTO:
            return parse_jump_stmt();

        case tLBRACE:
            return parse_compound_stmt();

        case kIF:
        case kSWITCH:
            return parse_selection_stmt();

        case kWHILE:
        case kFOR:
            return parse_iteration_stmt();

        case kCASE:
        case kDEFAULT:
            return parse_labeled_stmt();
    }

    if (match_token2(tIDENT, tCOLON)) return parse_labeled_stmt();

    return parse_expression_stmt();
}

Vector *parse_parameter_list()
{
    Vector *params = new_vector();

    if (match_token(tRPAREN)) return params;

    while (1) {
        Type *type = parse_type_name();
        char *name = expect_token(tIDENT)->sval;

        vector_push_back(params, new_var_decl_ast(AST_LVAR_DECL, type, name));
        if (match_token(tRPAREN)) break;
        expect_token(tCOMMA);
    }

    return params;
}

AST *parse_external_declaration()
{
    char *fname;
    Vector *params;
    AST *func;
    Type *ret_type;

    SAVE_TOKENSEQ;

    ret_type = NULL;
    if (match_type_specifier()) ret_type = parse_type_name();
    if (ret_type == NULL)
        // warn("returning type is deduced to int", __FILE__, __LINE__);
        ret_type = type_int();

    if (!match_token2(tIDENT, tLPAREN)) {  // global variable
        RESTORE_TOKENSEQ;
        return parse_var_init_declaration(AST_GVAR_DECL);
    }

    fname = expect_token(tIDENT)->sval;

    expect_token(tLPAREN);
    params = parse_parameter_list();
    expect_token(tRPAREN);

    // if semicolon follows the function parameter list,
    // it's a function declaration.
    if (match_token(tSEMICOLON)) {
        pop_token();
        return new_func_ast(AST_FUNC_DECL, fname, NULL, params, ret_type);
    }

    func = new_func_ast(AST_FUNCDEF, fname, NULL, params, ret_type);
    func->body = parse_compound_stmt();

    return func;
}

Vector *parse_prog(Vector *tokens)
{
    Vector *asts;

    asts = new_vector();
    init_tokenseq(tokens);

    while (peek_token()->kind != tEOF) {
        AST *ast;

        ast = parse_external_declaration();
        vector_push_back(asts, ast);
    }

    return asts;
}
