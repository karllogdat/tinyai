#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdlib.h>
#include "ast_node.h"
#include "token.h"

typedef struct Parser {
        struct TokenList *toks;
        size_t curr;
        bool has_error;
        bool panic_mode;
} Parser;

Parser *parser_create(struct TokenList *toks);
void parser_free(Parser *p);

ASTNode *parse(struct TokenList *toks);

#endif