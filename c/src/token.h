#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>
#include "transition_table.h"

/**
 * Token structure representing a lexical token.
 *
 * Members:
 * - type: The type of the token (from TokenType enum).
 * - lexeme: The string representation of the token.
 * - line: The line number where the token appears.
 * - col: The column number where the token appears.
 */
struct Token {
        TokenType type;
        char *lexeme;

        int line;
        int col;
};

/**
 * Creates a new pointer to a dynamically allocated Token.
 * Requires freeing with token_destroy().
 */
struct Token *
token_create(TokenType type, const char *lexeme, int line, int col);

/**
 * Frees a dynamically allocated Token.
 */
void token_destroy(struct Token *token);

/**
 * Dynamically allocated list of Tokens.
 * Requires freeing with token_list_destroy().
 *
 * Members:
 * - tokens: Array of pointers to Token.
 * - size: Current number of tokens in the list.
 * - capacity: Current allocated capacity of the list.
 */
struct TokenList {
        struct Token **tokens;
        size_t size;
        size_t capacity;
};

/**
 * Creates a new pointer to a dynamically allocated TokenList.
 * See token_list_destroy() for freeing.
 *
 * Returns a pointer to the created TokenList, or NULL on failure.
 */
struct TokenList *token_list_create();

/**
 * Frees a dynamically allocated TokenList and all its Tokens.
 */
void token_list_destroy(struct TokenList *list);

/**
 * Inserts a Token pointer into the TokenList.
 *
 * Returns the index of the inserted token, or -1 on failure.
 */
int token_list_insert(struct TokenList *list, struct Token *token);

/**
 * Retrieves a Token pointer from the TokenList by index.
 *
 * Returns the Token pointer at the given index, or NULL if out of bounds.
 */
struct Token *token_list_get(struct TokenList *list, size_t index);

#endif