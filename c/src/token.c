#include "token.h"
#include "transition_table.h"
#include <string.h>
#include <stdlib.h>

struct Token *token_create(TokenType type, const char *lexeme, int line, int col)
{
    struct Token *token = malloc(sizeof(struct Token));
    if (!token)
    {
        return NULL;
    }

    token->type = type;
    token->lexeme = strdup(lexeme);
    token->line = line;
    token->col = col;

    return token;
}

void token_destroy(struct Token *token)
{
    if (!token)
        return;

    free(token->lexeme);
    free(token);
}

struct TokenList *token_list_create()
{
    struct TokenList *list = malloc(sizeof(struct TokenList));
    if (!list)
        return NULL;

    list->size = 0;
    list->capacity = 16; // initial capacity, will change as needed
    list->tokens = malloc(list->capacity * sizeof(struct Token *));
    if (!list->tokens)
    {
        free(list);
        return NULL;
    }

    return list;
}

void token_list_destroy(struct TokenList *list)
{
    if (!list)
        return;

    for (size_t i = 0; i < list->size; i++)
    {
        token_destroy(list->tokens[i]);
    }
    free(list->tokens);
    free(list);
}

int token_list_insert(struct TokenList *list, struct Token *token)
{
    if (!list || !token)
        return -1;

    if (list->size >= list->capacity)
    {
        list->capacity = list->capacity * 2;
        struct Token **new_toks = realloc(list->tokens, list->capacity * sizeof(struct Token *));

        if (!new_toks)
            return -1;

        list->tokens = new_toks;
    }

    list->tokens[list->size++] = token;
    return list->size - 1;
}

struct Token *token_list_get(struct TokenList *list, size_t idx)
{
    if (!list || idx >= list->size)
        return NULL;

    return list->tokens[idx];
}