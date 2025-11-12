#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>
#include "transition_table.h"

struct Token
{
    TokenType type;
    char *lexeme;

    int line;
    int col;
};

struct Token *token_create(TokenType type, const char *lexeme, int line, int col);
void token_destroy(struct Token *token);

struct TokenList
{
    struct Token **tokens;
    size_t size;
    size_t capacity;
};

struct TokenList *token_list_create();
void token_list_destroy(struct TokenList *list);
int token_list_insert(struct TokenList *list, struct Token *token);
struct Token *token_list_get(struct TokenList *list, size_t index);

#endif