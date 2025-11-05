#ifndef LEXER_H
#define LEXER_H

#include "transition_table.h"

struct Token
{
    TokenType type;
    char *lexeme;
};

char *token_type_to_string(TokenType type);

struct Lexer
{
    char *source_code;
    size_t position;

    FILE *symbol_table_file;
};

void lexer_init(struct Lexer *lexer, char *source);
void lex(struct Lexer *lexer);

#endif