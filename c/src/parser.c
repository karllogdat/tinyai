#include "parser.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast_node.h"
#include "token.h"

/* parser lifetime handling */

Parser *parser_create(struct TokenList *toks)
{
        if (!toks) {
                fprintf(stderr, "null tokenlist in parser_create\n");
                return NULL;
        }

        Parser *p = malloc(sizeof(Parser));
        if (!p) {
                fprintf(stderr, "malloc failed for parser\n");
                return NULL;
        }

        p->toks = toks;
        p->curr = 0;
        p->has_error = false;
        p->panic_mode = false;

        return p;
}

void parser_free(Parser *p)
{
        if (!p) {
                return;
        }

        free(p);
}

/* helper functions */

static struct Token *curr(Parser *p)
{
        return token_list_get(p->toks, p->curr);
}

static struct Token *prev(Parser *p)
{
        if (p->curr == 0) {
                return NULL;
        }
        return token_list_get(p->toks, p->curr - 1);
}

static bool is_at_end(Parser *p)
{
        struct Token *tok = curr(p);
        return tok == NULL;
}

/**
 * get current token and advance parser current
 */
static struct Token *advance(Parser *p)
{
        if (!is_at_end(p)) {
                p->curr++;
        }

        return prev(p);
}

/**
 * checks current token if it matches given tokentype
 */
static bool check(Parser *p, TokenType type)
{
        if (is_at_end(p)) {
                return false;
        }
        struct Token *tok = curr(p);
        return tok->type == type;
}

/**
 * consume current tok if current type matches
 * given tokentype
 */
static bool match(Parser *p, TokenType type)
{
        if (check(p, type)) {
                advance(p);
                return true;
        }
        return false;
}

/**
 * match multiple token types
 */
static bool match_any(Parser *p, int count, ...)
{
        va_list args;
        va_start(args, count);

        for (int i = 0; i < count; i++) {
                TokenType type = va_arg(args, TokenType);
                if (check(p, type)) {
                        advance(p);
                        va_end(args);
                        return true;
                }
        }

        va_end(args);
        return false;
}

/**
 * consume expected token, report error otherwise
 */
static struct Token *consume(Parser *p, TokenType type, const char *msg)
{
        if (check(p, type)) {
                return advance(p);
        }

        struct Token *tok = curr(p);
        fprintf(stderr,
                "parse error at line %d, col %d: %s.\n",
                tok ? tok->line : -1,
                tok ? tok->col : -1,
                msg);
        p->has_error = true;
        return NULL;
}

static void err_at_curr(Parser *p, const char *msg)
{
        if (p->panic_mode) {
                return;
        }
        p->panic_mode = true;
        p->has_error = true;

        struct Token *tok = curr(p);
        fprintf(stderr,
                "parse error at line %d, col %d: %s. got '%s' of type '%s'.\n",
                tok ? tok->line : -1,
                tok ? tok->col : -1,
                msg,
                tok ? tok->lexeme : "NULL",
                tok ? tok_type_to_str(tok->type) : "NULL");
}

static void synchronize(Parser *p)
{
        p->panic_mode = false;

        while (!is_at_end(p)) {
                if (prev(p)->type == SEMI_COLON)
                        return;

                switch (curr(p)->type) {
                case IF_TOK:
                case ELIF_TOK:
                case ELSE_TOK:
                case WHILE_TOK:
                case FOR_TOK:
                case PRINT_TOK:
                case INT_TOK:
                case FLOAT_TOK:
                case BOOL_TOK:
                case CHAR_TOK:
                case STRING_TOK:
                case LEFT_CURLY_BRACE:
                case RIGHT_CURLY_BRACE:
                        return;
                default:
                        break;
                }
                advance(p);
        }
}

static bool is_stmt_start(Parser *p)
{
        TokenType type = curr(p)->type;
        return type == IF_TOK || type == WHILE_TOK || type == FOR_TOK ||
            type == PRINT_TOK || type == LEFT_CURLY_BRACE ||
            type == SEMI_COLON || type == IDENTIFIER || type == INT_TOK ||
            type == FLOAT_TOK || type == BOOL_TOK || type == CHAR_TOK ||
            type == STRING_TOK;
}

/* program level non-terminal forward declarations */

static ASTNode *parse_stmt(Parser *p);
static ASTNode *parse_expr(Parser *p);

/* non-terminal functions */

static ASTNode *parse_program(Parser *p)
{
        StmtListNode *stmts = stmt_list_create();
        if (!stmts)
                return NULL;

        while (!is_at_end(p)) {
                ASTNode *stmt = parse_stmt(p);
                if (stmt) {
                        stmt_list_add(stmts, stmt);
                }
                if (p->has_error) {
                        synchronize(p);
                }
        }

        return node_program_create(stmts);
}

static ASTNode *parse_stmt_block(Parser *p)
{
        if (!consume(p,
                     LEFT_CURLY_BRACE,
                     "expected '{' at start of statement block"))
                return NULL;

        StmtListNode *stmts = stmt_list_create();
        if (!stmts)
                return NULL;

        while (!check(p, RIGHT_CURLY_BRACE) && !is_at_end(p)) {
                ASTNode *stmt = parse_stmt(p);
                if (stmt) {
                        stmt_list_add(stmts, stmt);
                }
                if (p->has_error) {
                        synchronize(p);
                }
        }

        if (!consume(p,
                     RIGHT_CURLY_BRACE,
                     "expected '}' at end of statement block")) {
                stmt_list_free(stmts);
                return NULL;
        }

        return node_stmt_block_create(stmts);
}

static DataType parse_type(Parser *p)
{
        if (match(p, INT_TOK))
                return TYPE_INT;
        if (match(p, FLOAT_TOK))
                return TYPE_FLOAT;
        if (match(p, BOOL_TOK))
                return TYPE_BOOL;
        if (match(p, CHAR_TOK))
                return TYPE_CHAR;
        if (match(p, STRING_TOK))
                return TYPE_STRING;

        err_at_curr(p, "expected type specifier");
        return TYPE_INT; // fallback type
}

static ASTNode *parse_decl(Parser *p)
{
        DataType type = parse_type(p);

        struct Token *ident_tok =
            consume(p, IDENTIFIER, "expected identifier in declaration");
        if (!ident_tok)
                return NULL;

        char *ident = ident_tok->lexeme;
        ASTNode *init_expr = NULL;

        if (match(p, ASSIGN)) {
                init_expr = parse_expr(p);
                if (!init_expr)
                        return NULL;
        }

        return node_decl_create(type, ident, init_expr);
}

static ASTNode *parse_assign(Parser *p)
{
        struct Token *ident_tok =
            consume(p, IDENTIFIER, "expected identifier in assignment");
        if (!ident_tok)
                return NULL;

        if (!consume(p, ASSIGN, "expected '=' in assignment")) {
                return NULL;
        }

        if (match(p, INPUT_TOK)) {
                if (!consume(p, LEFT_PARENTHESIS, "expected ')' after 'input'"))
                        return NULL;

                struct Token *prompt_tok =
                    consume(p,
                            STRING_LITERAL,
                            "expected string literal for input prompt");
                if (!prompt_tok)
                        return NULL;

                if (!consume(p,
                             RIGHT_PARENTHESIS,
                             "expected ')' after input prompt"))
                        return NULL;

                return node_input_assign_create(ident_tok->lexeme,
                                                prompt_tok->lexeme);
        }

        ASTNode *expr = parse_expr(p);
        if (!expr)
                return NULL;

        return node_assign_create(ident_tok->lexeme, expr);
}

static ASTNode *parse_if_stmt(Parser *p)
{
        if (!consume(p, LEFT_PARENTHESIS, "expected '(' after 'if'")) {
                return NULL;
        }

        ASTNode *cond = parse_expr(p);
        if (!cond) {
                return NULL;
        }

        if (!consume(p, RIGHT_PARENTHESIS, "expected ')' after if condition")) {
                ast_node_free(cond);
                return NULL;
        }

        ASTNode *if_body = parse_stmt(p);
        if (!if_body) {
                ast_node_free(cond);
                return NULL;
        }

        // used if has succeeding elif tokens
        ElifNode *elif_list = NULL;
        ElifNode *elif_tail = NULL;
        while (match(p, ELIF_TOK)) {
                if (!consume(
                        p, LEFT_PARENTHESIS, "expected '(' after 'elif'")) {
                        ast_node_free(cond);
                        ast_node_free(if_body);
                        elif_list_free(elif_list);
                        return NULL;
                }

                ASTNode *elif_cond = parse_expr(p);
                if (!elif_cond) {
                        ast_node_free(cond);
                        ast_node_free(if_body);
                        elif_list_free(elif_list);
                        return NULL;
                }

                if (!consume(
                        p, RIGHT_PARENTHESIS, "expected ')' after 'elif'")) {
                        ast_node_free(cond);
                        ast_node_free(if_body);
                        elif_list_free(elif_list);
                        return NULL;
                }

                ASTNode *elif_body = parse_stmt(p);
                if (!elif_body) {
                        ast_node_free(cond);
                        ast_node_free(if_body);
                        ast_node_free(elif_cond);
                        elif_list_free(elif_list);
                        return NULL;
                }

                ElifNode *new_elif =
                    elif_node_create(elif_cond, elif_body, NULL);
                if (!new_elif) {
                        ast_node_free(cond);
                        ast_node_free(if_body);
                        ast_node_free(elif_cond);
                        ast_node_free(elif_body);
                        elif_list_free(elif_list);
                        return NULL;
                }

                if (!elif_list) {
                        elif_list = new_elif;
                        elif_tail = new_elif;
                } else {
                        elif_tail->next = new_elif;
                        elif_tail = new_elif;
                }
        }

        ASTNode *else_body = NULL;
        if (match(p, ELSE_TOK)) {
                else_body = parse_stmt(p);
                if (!else_body) {
                        ast_node_free(cond);
                        ast_node_free(if_body);
                        elif_list_free(elif_list);
                        return NULL;
                }
        }

        return node_if_create(cond, if_body, elif_list, else_body);
}

static ASTNode *parse_while(Parser *p)
{
        if (!consume(p, LEFT_PARENTHESIS, "expected '(' after 'while'")) {
                return NULL;
        }

        ASTNode *cond = parse_expr(p);
        if (!cond) {
                return NULL;
        }

        if (!consume(
                p, RIGHT_PARENTHESIS, "expected ')' after while condition")) {
                ast_node_free(cond);
                return NULL;
        }

        ASTNode *body = parse_stmt(p);
        if (!body) {
                ast_node_free(cond);
                return NULL;
        }

        return node_while_create(cond, body);
}

static ASTNode *parse_for(Parser *p)
{
        if (!consume(p, LEFT_PARENTHESIS, "expected '(' after 'for'")) {
                return NULL;
        }

        ASTNode *init = NULL;
        if (match(p, SEMI_COLON)) {
                init = NULL; // no initializer stmt
        } else if (check(p, INT_TOK) || check(p, FLOAT_TOK) ||
                   check(p, BOOL_TOK) || check(p, CHAR_TOK) ||
                   check(p, STRING_TOK)) {
                init = parse_decl(p);
                if (!init)
                        return NULL;
                if (!consume(
                        p, SEMI_COLON, "expected ';' after for initializer")) {
                        ast_node_free(init);
                        return NULL;
                }
        } else {
                // if not empty and not starting with keyword => assume
                // assignment
                // struct Token *ident = advance(p);
                init = parse_assign(p);
                if (!init)
                        return NULL;
                if (!consume(
                        p, SEMI_COLON, "expected ';' after for initializer")) {
                        ast_node_free(init);
                        return NULL;
                }
        }

        ASTNode *cond = NULL;
        if (!check(p, SEMI_COLON)) {
                cond = parse_expr(p);
                if (!cond) {
                        ast_node_free(init);
                        return NULL;
                }
        }

        if (!consume(p, SEMI_COLON, "expected ';' after for condition")) {
                ast_node_free(init);
                ast_node_free(cond);
                return NULL;
        }

        ASTNode *iter = NULL;
        if (!check(p, RIGHT_PARENTHESIS)) {
                if (check(p, IDENTIFIER)) {
                        struct Token *next =
                            token_list_get(p->toks, p->curr + 1);
                        if (next && next->type == ASSIGN) {
                                // TODO: SKETCHY, DOUBLE CHECK IF WORKING
                                // advance(p); // consume ident
                                iter = parse_assign(p);
                        } else {
                                iter = parse_expr(p);
                        }
                } else {
                        iter = parse_expr(p);
                }

                if (!iter) {
                        ast_node_free(cond);
                        ast_node_free(init);
                        return NULL;
                }
        }

        if (!consume(
                p, RIGHT_PARENTHESIS, "expected ')' after for iteration")) {
                ast_node_free(init);
                ast_node_free(cond);
                ast_node_free(iter);
                return NULL;
        }

        ASTNode *body = parse_stmt(p);
        if (!body) {
                ast_node_free(init);
                ast_node_free(cond);
                ast_node_free(iter);
                return NULL;
        }

        return node_for_create(init, cond, iter, body);
}

static ASTNode *parse_print(Parser *p)
{
        if (!consume(p, LEFT_PARENTHESIS, "expected '(' after 'print'")) {
                return NULL;
        }

        ASTNode *expr = parse_expr(p);
        if (!expr) {
                return NULL;
        }

        if (!consume(p, RIGHT_PARENTHESIS, "expected ')' after 'print'")) {
                ast_node_free(expr);
                return NULL;
        }

        return node_print_create(expr);
}

static ASTNode *parse_stmt(Parser *p)
{
        // printf(
        //     "parsing statement at token: %s of type: %s at line %d, col
        //     %d\n", curr(p) ? curr(p)->lexeme : "NULL",
        //     tok_type_to_str(curr(p) ? curr(p)->type : -1),
        //     curr(p)->line,
        //     curr(p)->col);

        // remember empty statements
        if (match(p, SEMI_COLON)) {
                return NULL;
        }

        if (check(p, LEFT_CURLY_BRACE)) {
                return parse_stmt_block(p);
        }

        if (match(p, IF_TOK)) {
                ASTNode *res = parse_if_stmt(p);
                if (!res) {
                        synchronize(p);
                }
                return res;
        }

        if (match(p, WHILE_TOK)) {
                ASTNode *res = parse_while(p);
                if (!res) {
                        synchronize(p);
                }
                return res;
        }

        if (match(p, FOR_TOK)) {
                ASTNode *res = parse_for(p);
                if (!res) {
                        synchronize(p);
                }
                return res;
        }

        // semicolon terminated statements
        if (match(p, PRINT_TOK)) {
                ASTNode *print = parse_print(p);
                if (!print) {
                        synchronize(p);
                        return NULL;
                }
                if (!consume(
                        p, SEMI_COLON, "expected ';' after print statement")) {
                        ast_node_free(print);
                        synchronize(p);
                        return NULL;
                }
                return print;
        }

        // check types for decl
        if (check(p, INT_TOK) || check(p, FLOAT_TOK) || check(p, BOOL_TOK) ||
            check(p, CHAR_TOK) || check(p, STRING_TOK)) {
                ASTNode *decl = parse_decl(p);
                if (!decl) {
                        synchronize(p);
                        return NULL;
                }
                if (!consume(p, SEMI_COLON, "expected ';' after declaration")) {
                        ast_node_free(decl);
                        synchronize(p);
                        return NULL;
                }
                return decl;
        }

        // check next for assign token to differentiate vs expr
        if (check(p, IDENTIFIER)) {
                struct Token *next = token_list_get(p->toks, p->curr + 1);
                if (next && next->type == ASSIGN) {
                        // TODO: SKETCHY, CHECK IF PROPER
                        // advance(p);
                        ASTNode *assign = parse_assign(p);
                        if (!assign) {
                                synchronize(p);
                                return NULL;
                        }
                        if (!consume(p,
                                     SEMI_COLON,
                                     "expected ';' after assignment")) {
                                ast_node_free(assign);
                                synchronize(p);
                                return NULL;
                        }
                        return assign;
                } else {
                        ASTNode *expr = parse_expr(p);
                        if (!expr) {
                                synchronize(p);
                                return NULL;
                        }
                        if (!consume(
                                p,
                                SEMI_COLON,
                                "expected ';' after expression statement")) {
                                ast_node_free(expr);
                                synchronize(p);
                                return NULL;
                        }
                        return expr;
                }
        }

        if (!is_stmt_start(p)) {
                if (check(p, ELIF_TOK) || check(p, ELSE_TOK)) {
                        err_at_curr(p,
                                    "unexpected 'elif' or 'else' without "
                                    "preceding 'if'");
                        advance(p);
                } else {
                        err_at_curr(p, "expected statement");
                        advance(p);
                }
                synchronize(p);
                return NULL;
        }

        err_at_curr(p, "expected statement");
        synchronize(p);
        return NULL;
}

/**
 * expression parsing forward declarations
 */

static ASTNode *parse_lor(Parser *p);
static ASTNode *parse_land(Parser *p);
static ASTNode *parse_eq(Parser *p);
static ASTNode *parse_rel(Parser *p);
static ASTNode *parse_add(Parser *p);
static ASTNode *parse_mult(Parser *p);
static ASTNode *parse_pow(Parser *p);
static ASTNode *parse_unary(Parser *p);
static ASTNode *parse_primary(Parser *p);

static ASTNode *parse_lor(Parser *p)
{
        ASTNode *left = parse_land(p);
        if (!left) {
                return NULL;
        }

        while (match(p, OR)) {
                ASTNode *right = parse_land(p);
                if (!right) {
                        ast_node_free(left);
                        return NULL;
                }
                left = node_binary_op_create(OP_OR, left, right);
                if (!left) {
                        ast_node_free(right);
                        return NULL;
                }
        }

        return left;
}

static ASTNode *parse_land(Parser *p)
{
        ASTNode *left = parse_eq(p);
        if (!left) {
                return NULL;
        }

        while (match(p, AND)) {
                ASTNode *right = parse_eq(p);
                if (!right) {
                        ast_node_free(left);
                        return NULL;
                }
                left = node_binary_op_create(OP_AND, left, right);
                if (!left) {
                        ast_node_free(right);
                        return NULL;
                }
        }

        return left;
}

static ASTNode *parse_eq(Parser *p)
{
        ASTNode *left = parse_rel(p);
        if (!left) {
                return NULL;
        }

        while (true) {
                Operator op;
                if (match(p, EQUAL)) {
                        op = OP_EQ;
                } else if (match(p, NOT_EQUAL)) {
                        op = OP_NEQ;
                } else {
                        break;
                }

                ASTNode *right = parse_rel(p);
                if (!right) {
                        ast_node_free(left);
                        return NULL;
                }

                left = node_binary_op_create(op, left, right);
                if (!left) {
                        ast_node_free(right);
                        return NULL;
                }
        }

        return left;
}

static ASTNode *parse_rel(Parser *p)
{
        ASTNode *left = parse_add(p);
        if (!left) {
                return NULL;
        }

        while (true) {
                Operator op;
                if (match(p, LESS_THAN)) {
                        op = OP_LT;
                } else if (match(p, LESS_EQUAL)) {
                        op = OP_LTEQ;
                } else if (match(p, GREATER_THAN)) {
                        op = OP_GT;
                } else if (match(p, GREATER_EQUAL)) {
                        op = OP_GTEQ;
                } else {
                        break;
                }

                ASTNode *right = parse_add(p);
                if (!right) {
                        ast_node_free(left);
                        return NULL;
                }
                left = node_binary_op_create(op, left, right);
                if (!left) {
                        ast_node_free(right);
                        return NULL;
                }
        }

        return left;
}

static ASTNode *parse_add(Parser *p)
{
        ASTNode *left = parse_mult(p);
        if (!left) {
                return NULL;
        }

        while (true) {
                Operator op;
                if (match(p, PLUS)) {
                        op = OP_ADD;
                } else if (match(p, MINUS)) {
                        op = OP_SUB;
                } else {
                        break;
                }

                ASTNode *right = parse_mult(p);
                if (!right) {
                        ast_node_free(left);
                        return NULL;
                }
                left = node_binary_op_create(op, left, right);
                if (!left) {
                        ast_node_free(right);
                        return NULL;
                }
        }

        return left;
}

static ASTNode *parse_mult(Parser *p)
{
        ASTNode *left = parse_pow(p);
        if (!left) {
                return NULL;
        }

        while (true) {
                Operator op;
                if (match(p, ASTERISK)) {
                        op = OP_MUL;
                } else if (match(p, SLASH)) {
                        op = OP_DIV;
                } else if (match(p, MODULO)) {
                        op = OP_MOD;
                } else if (match(p, DOUBLE_SLASH)) {
                        op = OP_INTDIV;
                } else {
                        break;
                }

                ASTNode *right = parse_pow(p);
                if (!right) {
                        ast_node_free(left);
                        return NULL;
                }
                left = node_binary_op_create(op, left, right);
                if (!left) {
                        ast_node_free(right);
                        return NULL;
                }
        }

        return left;
}

static ASTNode *parse_pow(Parser *p)
{
        ASTNode *left = parse_unary(p);
        if (!left) {
                return NULL;
        }

        // (**) is right associative, handle using recursion
        // for ease. not efficient for deep exponentiation
        if (match(p, DOUBLE_ASTERISK)) {
                ASTNode *right = parse_pow(p);
                if (!right) {
                        ast_node_free(left);
                        return NULL;
                }
                left = node_binary_op_create(OP_POW, left, right);
                if (!left) {
                        ast_node_free(right);
                        return NULL;
                }
        }

        return left;
}

static ASTNode *parse_unary(Parser *p)
{
        /* handles rule using recursion since unary exprs are less
           likely to nest */
        if (match(p, NOT)) {
                ASTNode *operand = parse_unary(p);
                if (!operand) {
                        return NULL;
                }
                return node_unary_op_create(OP_NOT, operand);
        }

        if (match(p, MINUS)) {
                ASTNode *operand = parse_unary(p);
                if (!operand) {
                        return NULL;
                }
                return node_unary_op_create(OP_NEG, operand);
        }

        return parse_primary(p);
}

static ASTNode *parse_primary(Parser *p)
{
        if (match(p, INT_LITERAL)) {
                LiteralValue val;
                val.int_val = atoi(prev(p)->lexeme);
                return node_literal_create(TYPE_INT, val);
        }

        if (match(p, FLOAT_LITERAL)) {
                LiteralValue val;
                val.float_val = atof(prev(p)->lexeme);
                return node_literal_create(TYPE_FLOAT, val);
        }

        if (match(p, BOOL_LITERAL)) {
                LiteralValue val;
                val.bool_val = strcmp(prev(p)->lexeme, "true") == 0;
                return node_literal_create(TYPE_BOOL, val);
        }

        if (match(p, CHAR_LITERAL)) {
                LiteralValue val;
                // remember char lexeme is stored as 'c'
                val.char_val = prev(p)->lexeme[1];
                return node_literal_create(TYPE_CHAR, val);
        }

        if (match(p, STRING_LITERAL)) {
                LiteralValue val;
                size_t len = strlen(prev(p)->lexeme);
                // Allocate memory for the string (excluding quotes)
                val.str_val =
                    malloc(len - 1); // -2 for quotes, +1 for null terminator
                if (!val.str_val) {
                        fprintf(stderr, "malloc failed for string literal\n");
                        return NULL;
                }
                strncpy(val.str_val, prev(p)->lexeme + 1, len - 2);
                val.str_val[len - 2] = '\0';
                return node_literal_create(TYPE_STRING, val);
        }

        // differentiate between normal identifier vs func-call
        if (match(p, IDENTIFIER)) {
                char *name = prev(p)->lexeme;

                if (match(p, LEFT_PARENTHESIS)) {
                        ArgNode *args = NULL;
                        ArgNode *arg_tail = NULL;

                        if (!check(p, RIGHT_PARENTHESIS)) {
                                // first argument
                                ASTNode *arg_expr = parse_expr(p);
                                if (!arg_expr) {
                                        return NULL;
                                }

                                args = arg_node_create(arg_expr, NULL);
                                arg_tail = args;

                                // remaining arguments
                                while (match(p, COMMA)) {
                                        arg_expr = parse_expr(p);
                                        if (!arg_expr)
                                                return NULL;

                                        ArgNode *new_arg =
                                            arg_node_create(arg_expr, NULL);
                                        arg_tail->next = new_arg;
                                        arg_tail = new_arg;
                                }
                        }

                        if (!consume(p,
                                     RIGHT_PARENTHESIS,
                                     "expected ')' after arguments")) {
                                arg_list_free(args);
                                return NULL;
                        }

                        return node_func_call_create(name, args);
                }

                // just ident
                return node_ident_create(name);
        }

        // parenthesized exprs
        if (match(p, LEFT_PARENTHESIS)) {
                ASTNode *expr = parse_expr(p);
                if (!expr) {
                        return NULL;
                }

                if (!consume(p,
                             RIGHT_PARENTHESIS,
                             "expected ')' after expression")) {
                        ast_node_free(expr);
                        return NULL;
                }

                return expr;
        }

        err_at_curr(p, "expected expression");
        return NULL;
}

static ASTNode *parse_expr(Parser *p)
{
        return parse_lor(p);
}

ASTNode *parse(struct TokenList *toks)
{
        Parser *p = parser_create(toks);
        if (!p) {
                return NULL;
        }

        ASTNode *ast = parse_program(p);
        bool has_error = p->has_error;
        parser_free(p);

        if (has_error) {
                ast_node_free(ast);
                return NULL;
        }

        return ast;
}