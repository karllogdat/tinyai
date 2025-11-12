#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "transition_table.h"

struct Lexer {
        struct TokenList *tokens;
        char *source_code;
        size_t position;

        FILE *symbol_table_file;
};

void lexer_init(struct Lexer *lexer, char *source);
void lexer_lex(struct Lexer *lexer);

void lexer_print_toks(struct Lexer *lx);

#endif