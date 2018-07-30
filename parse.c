#include "aqcc.h"

AST *parse_expr(TokenSeq *tokseq);
AST *parse_assignment_expr(TokenSeq *tokseq);
AST *parse_stmt(TokenSeq *tokseq);

TokenSeq *new_token_seq(Vector *tokens)
{
    TokenSeq *this = safe_malloc(sizeof(TokenSeq));

    this->tokens = tokens;
    this->idx = 0;
    return this;
}

Token *peek_token(TokenSeq *seq)
{
    Token *token = vector_get(seq->tokens, seq->idx);
    if (token == NULL) error("no next token.", __FILE__, __LINE__);
    return token;
}

Token *pop_token(TokenSeq *seq)
{
    Token *token = vector_get(seq->tokens, seq->idx++);
    if (token == NULL) error("no next token.", __FILE__, __LINE__);
    return token;
}

Token *expect_token(TokenSeq *seq, int kind)
{
    Token *token = pop_token(seq);
    if (token->kind != kind) error("unexpected token.", __FILE__, __LINE__);
    return token;
}

int match_token(TokenSeq *seq, int kind)
{
    Token *token = peek_token(seq);
    return token->kind == kind;
}

int match_token2(TokenSeq *seq, int kind0, int kind1)
{
    Token *token = peek_token(seq);
    if (token->kind != kind0) return 0;
    seq->idx++;
    token = peek_token(seq);
    seq->idx--;
    if (token->kind != kind1) return 0;
    return 1;
}

TokenSeqSaved *new_token_seq_saved(TokenSeq *tokseq)
{
    TokenSeqSaved *this;

    this = (TokenSeqSaved *)safe_malloc(sizeof(TokenSeqSaved));
    this->idx = tokseq->idx;

    return this;
}

void restore_token_seq_saved(TokenSeqSaved *saved, TokenSeq *tokseq)
{
    tokseq->idx = saved->idx;
}

#define SAVE_TOKSEQ \
    TokenSeqSaved *token_seq_saved__dummy = new_token_seq_saved(tokseq);
#define RESTORE_TOKSEQ restore_token_seq_saved(token_seq_saved__dummy, tokseq);

AST *parse_primary_expr(TokenSeq *tokseq)
{
    AST *ast;
    Token *token = pop_token(tokseq);
    switch (token->kind) {
        case tINT:
            ast = new_ast(AST_INT);
            ast->ival = token->ival;
            break;

        case tLPAREN:
            ast = parse_expr(tokseq);
            expect_token(tokseq, tRPAREN);
            break;

        case tIDENT:
            // ALL tIDENT should be handled in parse_postfix_expr()
            assert(0);

        default:
            error("unexpected token", __FILE__, __LINE__);
    }
    return ast;
}

Vector *parse_argument_expr_list(TokenSeq *tokseq)
{
    Vector *args = new_vector();

    if (match_token(tokseq, tRPAREN)) return args;

    while (1) {
        vector_push_back(args, parse_assignment_expr(tokseq));
        if (match_token(tokseq, tRPAREN)) break;
        expect_token(tokseq, tCOMMA);
    }

    return args;
}

AST *parse_postfix_expr(TokenSeq *tokseq)
{
    // TODO: should be recursive

    if (match_token2(tokseq, tIDENT, tLPAREN)) {  // function call
        Token *ident;
        AST *ast;

        ident = pop_token(tokseq);
        pop_token(tokseq);

        ast = new_func_ast(AST_FUNCCALL, ident->sval,
                           parse_argument_expr_list(tokseq), NULL, NULL);
        expect_token(tokseq, tRPAREN);
        return ast;
    }

    AST *ast;

    if (match_token(tokseq, tIDENT))  // variable
        ast = new_lvar_ast(pop_token(tokseq)->sval);
    else  // primary expr
        ast = parse_primary_expr(tokseq);

    while (1) {
        switch (peek_token(tokseq)->kind) {
            case tINC: {
                pop_token(tokseq);
                ast = new_unary_ast(AST_POSTINC, ast);
            } break;

            case tLBRACKET:
                pop_token(tokseq);
                ast = new_unary_ast(
                    AST_INDIR, new_binop_ast(AST_ADD, parse_expr(tokseq), ast));
                expect_token(tokseq, tRBRACKET);
                break;

            default:
                goto end;
        }
    }

end:
    return ast;
}

AST *parse_unary_expr(TokenSeq *tokseq)
{
    Token *token = peek_token(tokseq);
    switch (token->kind) {
        case tPLUS:
            pop_token(tokseq);
            return parse_postfix_expr(tokseq);

        case tMINUS:
            pop_token(tokseq);
            return new_unary_ast(AST_UNARY_MINUS, parse_unary_expr(tokseq));

        case tINC:
            pop_token(tokseq);
            return new_unary_ast(AST_PREINC, parse_unary_expr(tokseq));

        case tAND:
            pop_token(tokseq);
            return new_unary_ast(AST_ADDR, parse_unary_expr(tokseq));

        case tSTAR:
            pop_token(tokseq);
            return new_unary_ast(AST_INDIR, parse_unary_expr(tokseq));
    }

    return parse_postfix_expr(tokseq);
}

AST *parse_multiplicative_expr(TokenSeq *tokseq)
{
    AST *ast = parse_unary_expr(tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tSTAR:
                pop_token(tokseq);
                ast = new_binop_ast(AST_MUL, ast, parse_unary_expr(tokseq));
                break;

            case tSLASH:
                pop_token(tokseq);
                ast = new_binop_ast(AST_DIV, ast, parse_unary_expr(tokseq));
                break;

            case tPERCENT:
                pop_token(tokseq);
                ast = new_binop_ast(AST_REM, ast, parse_unary_expr(tokseq));
                break;

            default:
                return ast;
        }
    }
}

AST *parse_additive_expr(TokenSeq *tokseq)
{
    AST *lhs, *rhs;

    lhs = parse_multiplicative_expr(tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tPLUS:

                pop_token(tokseq);
                rhs = parse_multiplicative_expr(tokseq);
                lhs = new_binop_ast(AST_ADD, lhs, rhs);
                break;

            case tMINUS:
                pop_token(tokseq);
                rhs = parse_multiplicative_expr(tokseq);
                lhs = new_binop_ast(AST_SUB, lhs, rhs);
                break;

            default:
                return lhs;
        }
    }
}

AST *parse_shift_expr(TokenSeq *tokseq)
{
    AST *ast = parse_additive_expr(tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tLSHIFT:
                pop_token(tokseq);
                ast =
                    new_binop_ast(AST_LSHIFT, ast, parse_additive_expr(tokseq));
                break;
            case tRSHIFT:
                pop_token(tokseq);
                ast =
                    new_binop_ast(AST_RSHIFT, ast, parse_additive_expr(tokseq));
                break;
            default:
                return ast;
        }
    }
}

AST *parse_relational_expr(TokenSeq *tokseq)
{
    AST *ast = parse_shift_expr(tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tLT:
                pop_token(tokseq);
                ast = new_binop_ast(AST_LT, ast, parse_shift_expr(tokseq));
                break;

            case tGT:
                pop_token(tokseq);
                ast = new_binop_ast(AST_GT, ast, parse_shift_expr(tokseq));
                break;

            case tLTE:
                pop_token(tokseq);
                ast = new_binop_ast(AST_LTE, ast, parse_shift_expr(tokseq));
                break;

            case tGTE:
                pop_token(tokseq);
                ast = new_binop_ast(AST_GTE, ast, parse_shift_expr(tokseq));
                break;

            default:
                return ast;
        }
    }
}

AST *parse_equality_expr(TokenSeq *tokseq)
{
    AST *ast = parse_relational_expr(tokseq);

    while (1) {
        Token *token = peek_token(tokseq);
        switch (token->kind) {
            case tEQEQ:
                pop_token(tokseq);
                ast = new_binop_ast(AST_EQ, ast, parse_relational_expr(tokseq));
                break;

            case tNEQ:
                pop_token(tokseq);
                ast =
                    new_binop_ast(AST_NEQ, ast, parse_relational_expr(tokseq));
                break;

            default:
                return ast;
        }
    }
}

AST *parse_and_expr(TokenSeq *tokseq)
{
    AST *ast = parse_equality_expr(tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tAND) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_AND, ast, parse_equality_expr(tokseq));
    }
}

AST *parse_exclusive_or_expr(TokenSeq *tokseq)
{
    AST *ast = parse_and_expr(tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tHAT) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_XOR, ast, parse_and_expr(tokseq));
    }
}

AST *parse_inclusive_or_expr(TokenSeq *tokseq)
{
    AST *ast = parse_exclusive_or_expr(tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tBAR) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_OR, ast, parse_exclusive_or_expr(tokseq));
    }
}

AST *parse_logical_and_expr(TokenSeq *tokseq)
{
    AST *ast = parse_inclusive_or_expr(tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tANDAND) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_LAND, ast, parse_inclusive_or_expr(tokseq));
    }
}

AST *parse_logical_or_expr(TokenSeq *tokseq)
{
    AST *ast = parse_logical_and_expr(tokseq);

    while (1) {
        if (peek_token(tokseq)->kind != tBARBAR) return ast;
        pop_token(tokseq);
        ast = new_binop_ast(AST_LOR, ast, parse_logical_and_expr(tokseq));
    }
}

AST *parse_conditional_expr(TokenSeq *tokseq)
{
    AST *cond, *then, *els, *ast;

    cond = parse_logical_or_expr(tokseq);
    if (!match_token(tokseq, tQUESTION)) return cond;
    pop_token(tokseq);
    then = parse_expr(tokseq);
    expect_token(tokseq, tCOLON);
    els = parse_conditional_expr(tokseq);

    ast = new_ast(AST_COND);
    ast->cond = cond;
    ast->then = then;
    ast->els = els;
    return ast;
}

AST *parse_assignment_expr(TokenSeq *tokseq)
{
    AST *last, *rast;
    Token *token;
    int kind;

    SAVE_TOKSEQ;
    last = parse_unary_expr(tokseq);
    token = pop_token(tokseq);
    switch (token->kind) {
        case tEQ:
            kind = AST_NOP;
            break;

        case tPLUSEQ:
            kind = AST_ADD;
            break;

        case tMINUSEQ:
            kind = AST_SUB;
            break;

        case tSTAREQ:
            kind = AST_MUL;
            break;

        case tSLASHEQ:
            kind = AST_DIV;
            break;

        case tPERCENTEQ:
            kind = AST_REM;
            break;

        case tANDEQ:
            kind = AST_AND;
            break;

        case tHATEQ:
            kind = AST_XOR;
            break;

        case tBAREQ:
            kind = AST_OR;
            break;

        case tLSHIFTEQ:
            kind = AST_LSHIFT;
            break;

        case tRSHIFTEQ:
            kind = AST_RSHIFT;
            break;

        default:
            RESTORE_TOKSEQ;
            return parse_conditional_expr(tokseq);
    }

    rast = parse_assignment_expr(tokseq);
    if (kind != AST_NOP) rast = new_binop_ast(kind, last, rast);
    return new_binop_ast(AST_ASSIGN, last, rast);
}

AST *parse_expr(TokenSeq *tokseq) { return parse_assignment_expr(tokseq); }

AST *parse_expression_stmt(TokenSeq *tokseq)
{
    AST *expr = NULL;

    if (!match_token(tokseq, tSEMICOLON)) expr = parse_expr(tokseq);
    expect_token(tokseq, tSEMICOLON);

    return new_unary_ast(AST_EXPR_STMT, expr);
}

AST *parse_jump_stmt(TokenSeq *tokseq)
{
    Token *token = pop_token(tokseq);
    AST *ast = NULL;

    switch (token->kind) {
        case kRETURN:
            if (!match_token(tokseq, tSEMICOLON)) ast = parse_expr(tokseq);
            expect_token(tokseq, tSEMICOLON);
            ast = new_unary_ast(AST_RETURN, ast);
            break;

        case kBREAK:
            expect_token(tokseq, tSEMICOLON);
            ast = new_ast(AST_BREAK);
            break;

        case kCONTINUE:
            expect_token(tokseq, tSEMICOLON);
            ast = new_ast(AST_CONTINUE);
            break;

        default:
            error("unexpected token", __FILE__, __LINE__);
    }

    return ast;
}

AST *parse_iteration_stmt(TokenSeq *tokseq)
{
    Token *token = pop_token(tokseq);
    AST *ast;

    switch (token->kind) {
        case kWHILE: {
            AST *cond, *body;
            expect_token(tokseq, tLPAREN);
            cond = parse_expr(tokseq);
            expect_token(tokseq, tRPAREN);
            body = parse_stmt(tokseq);

            ast = new_while_stmt(cond, body);
        } break;

        case kFOR: {
            AST *initer, *cond, *iterer, *body;

            expect_token(tokseq, tLPAREN);
            if (match_token(tokseq, tSEMICOLON))
                initer = NULL;
            else
                initer = parse_expr(tokseq);
            expect_token(tokseq, tSEMICOLON);
            if (match_token(tokseq, tSEMICOLON))
                cond = NULL;
            else
                cond = parse_expr(tokseq);
            expect_token(tokseq, tSEMICOLON);
            if (match_token(tokseq, tRPAREN))
                iterer = NULL;
            else
                iterer = parse_expr(tokseq);
            expect_token(tokseq, tRPAREN);
            body = parse_stmt(tokseq);

            ast = new_ast(AST_FOR);
            ast->initer = initer;
            ast->midcond = cond;
            ast->iterer = iterer;
            ast->for_body = body;
        } break;

        default:
            error("unexpected token", __FILE__, __LINE__);
    }

    return ast;
}

Type *parse_type_name(TokenSeq *tokseq)
{
    Type *type;

    expect_token(tokseq, kINT);
    type = type_int();

    while (match_token(tokseq, tSTAR)) {
        pop_token(tokseq);
        type = new_pointer_type(type);
    }

    return type;
}

AST *parse_declaration(TokenSeq *tokseq)
{
    AST *ast;

    {
        Type *type;
        char *varname;

        type = parse_type_name(tokseq);
        varname = expect_token(tokseq, tIDENT)->sval;
        ast = new_lvar_decl_ast(type, varname);
    }

    while (match_token(tokseq, tLBRACKET)) {  // array
        Token *num;

        pop_token(tokseq);
        num = expect_token(tokseq, tINT);  // TODO: parse assignment-expr
        expect_token(tokseq, tRBRACKET);
        ast->type = new_array_type(ast->type, num->ival);
    }

    expect_token(tokseq, tSEMICOLON);

    return ast;
}

AST *parse_compound_stmt(TokenSeq *tokseq)
{
    AST *ast;
    Vector *stmts = new_vector();

    expect_token(tokseq, tLBRACE);
    while (!match_token(tokseq, tRBRACE)) {
        if (match_token(tokseq, kINT))
            vector_push_back(stmts, parse_declaration(tokseq));
        else
            vector_push_back(stmts, parse_stmt(tokseq));
    }
    expect_token(tokseq, tRBRACE);

    ast = new_ast(AST_COMPOUND);
    ast->stmts = stmts;

    return ast;
}

AST *parse_selection_stmt(TokenSeq *tokseq)
{
    AST *ast, *cond, *then, *els = NULL;

    expect_token(tokseq, kIF);
    expect_token(tokseq, tLPAREN);
    cond = parse_expr(tokseq);
    expect_token(tokseq, tRPAREN);
    then = parse_stmt(tokseq);
    if (match_token(tokseq, kELSE)) {
        pop_token(tokseq);
        els = parse_stmt(tokseq);
    }

    ast = new_ast(AST_IF);
    ast->cond = cond;
    ast->then = then;
    ast->els = els;
    return ast;
}

AST *parse_stmt(TokenSeq *tokseq)
{
    Token *token = peek_token(tokseq);

    switch (token->kind) {
        case kBREAK:
        case kCONTINUE:
        case kRETURN:
            return parse_jump_stmt(tokseq);

        case tLBRACE:
            return parse_compound_stmt(tokseq);

        case kIF:
            return parse_selection_stmt(tokseq);

        case kWHILE:
        case kFOR:
            return parse_iteration_stmt(tokseq);
    }

    return parse_expression_stmt(tokseq);
}

Vector *parse_parameter_list(TokenSeq *tokseq)
{
    Vector *params = new_vector();

    if (match_token(tokseq, tRPAREN)) return params;

    while (1) {
        Type *type = parse_type_name(tokseq);
        char *name = expect_token(tokseq, tIDENT)->sval;

        vector_push_back(params, new_lvar_decl_ast(type, name));
        if (match_token(tokseq, tRPAREN)) break;
        expect_token(tokseq, tCOMMA);
    }

    return params;
}

AST *parse_function_definition_or_declaration(TokenSeq *tokseq)
{
    char *fname;
    Vector *params;
    AST *func;
    Type *ret_type;

    ret_type = NULL;
    if (match_token(tokseq, kINT)) ret_type = parse_type_name(tokseq);
    if (ret_type == NULL)
        // warn("returning type is deduced to int", __FILE__, __LINE__);
        ret_type = type_int();

    fname = expect_token(tokseq, tIDENT)->sval;
    expect_token(tokseq, tLPAREN);
    params = parse_parameter_list(tokseq);
    expect_token(tokseq, tRPAREN);

    // if semicolon follows the function parameter list,
    // it's a function declaration.
    if (match_token(tokseq, tSEMICOLON)) {
        pop_token(tokseq);
        return new_func_ast(AST_FUNC_DECL, fname, NULL, params, ret_type);
    }

    func = new_func_ast(AST_FUNCDEF, fname, NULL, params, ret_type);
    func->body = parse_compound_stmt(tokseq);

    return func;
}

Vector *parse_prog(Vector *tokens)
{
    Vector *asts;
    TokenSeq *tokseq;

    asts = new_vector();
    tokseq = new_token_seq(tokens);

    while (peek_token(tokseq)->kind != tEOF) {
        AST *ast;

        ast = parse_function_definition_or_declaration(tokseq);
        vector_push_back(asts, ast);
    }

    return asts;
}
