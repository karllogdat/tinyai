#include "parser.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
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
        if (!is_at_end(p)) {
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
                advance(p);
        }

        struct Token *tok = curr(p);
        fprintf(stderr,
                "parse error at line %d, col %d: %s\n",
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
                "parse error at line %d, col %d: %s\n",
                tok ? tok->line : -1,
                tok ? tok->col : -1,
                msg);
}

static void synchronize(Parser *p)
{
        p->panic_mode = false;

        while (!is_at_end(p)) {
                if (prev(p)->type == SEMI_COLON)
                        return;

                switch (curr(p)->type) {
                case IF_TOK:
                case WHILE_TOK:
                case FOR_TOK:
                case PRINT_TOK:
                case INT_TOK:
                case FLOAT_TOK:
                case BOOL_TOK:
                case CHAR_TOK:
                case STRING_TOK:
                        return;
                default:
                        break;
                }
                advance(p);
        }
}

/* program level non-terminal forward declarations */

static ASTNode *parse_stmt(Parser *p);
static ASTNode *parse_expr(Parser *p);

/* non-terminal functions */

ASTNode *parse_program(Parser *p)
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

        while (!check(p, RIGHT_CURLY_BRACE) || !is_at_end(p)) {
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