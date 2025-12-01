#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"

static void lexer_clean(struct Lexer *l)
{
        struct TokenList *list = token_list_create();
        for (size_t i = 0; i < l->tokens->size; i++) {
                struct Token *tok = token_list_get(l->tokens, i);
                if (tok->type != WHITESPACE && tok->type != COMMENT &&
                    tok->type != MULTILINE_COMMENT) {
                        token_list_insert(list, tok);
                } else {
                        free(tok->lexeme);
                        free(tok);
                }
        }
        // TODO: free old list implementation
        // token_list_free(l->tokens);
        l->tokens = list;
}

void lexer_init(struct Lexer *lexer, char *source)
{
        lexer->source_code = source;
        lexer->position = 0;

        lexer->tokens = token_list_create();
        if (!lexer->tokens) {
                perror("Failed to create token list");
                exit(EXIT_FAILURE);
        }

        lexer->symbol_table_file = fopen("symbol_table.txt", "w");
        if (!lexer->symbol_table_file) {
                perror("Failed to open symbol table file");
                exit(EXIT_FAILURE);
        }
}

void lexer_lex(struct Lexer *lexer)
{
        int start_state = START_STATE_ID;
        size_t source_len = strlen(lexer->source_code);
        size_t current_pos = (size_t)lexer->position;
        size_t cur_line = 1;
        size_t cur_col = 1;

        while (current_pos < source_len) {
                int current_state = start_state;
                int last_accepting_state = -1;
                int last_accepting_pos = -1;
                size_t last_line = cur_line;
                size_t last_col = cur_col;

                size_t token_start_line = cur_line;
                size_t token_start_col = cur_col;

                size_t pos = current_pos;
                size_t line = cur_line;
                size_t col = cur_col;
                while (pos < source_len) {
                        unsigned char uc =
                            (unsigned char)lexer->source_code[pos];
                        int next_state = TRANSITION_TABLE[current_state][uc];

                        if (next_state == -1) {
                                break;
                        }

                        if (uc == '\n') {
                                line++;
                                col = 1;
                        } else {
                                col++;
                        }
                        current_state = next_state;

                        if (ACCEPT_STATE_IDS[current_state]) {
                                last_accepting_state = current_state;
                                last_accepting_pos = (int)pos;
                                last_line = line;
                                last_col = col;
                        }

                        pos++;
                }

                if (last_accepting_state != -1) {
                        size_t lexeme_length =
                            (size_t)last_accepting_pos - current_pos + 1;
                        char *lexeme = (char *)malloc(lexeme_length + 1);
                        memcpy(lexeme,
                               &lexer->source_code[current_pos],
                               lexeme_length);
                        lexeme[lexeme_length] = '\0';

                        TokenType token_type =
                            STATE_TOKEN_TYPE[last_accepting_state];

                        struct Token *token =
                            token_create(token_type,
                                         lexeme,
                                         (int)token_start_line,
                                         (int)token_start_col);
                        token_list_insert(lexer->tokens, token);

                        free(lexeme);

                        current_pos = (size_t)last_accepting_pos + 1;
                        lexer->position = (int)current_pos;
                        cur_line = last_line;
                        cur_col = last_col;
                } else {
                        struct Token *token = token_create(
                            UNKNOWN,
                            (char[]){ lexer->source_code[current_pos], '\0' },
                            (int)cur_line,
                            (int)cur_col);
                        token_list_insert(lexer->tokens, token);

                        current_pos++;
                        if (lexer->source_code[current_pos - 1] == '\n') {
                                cur_line++;
                                cur_col = 1;
                        } else {
                                cur_col++;
                        }
                        lexer->position = (int)current_pos;
                }
        }

        lexer_clean(lexer);
}

void lexer_print_toks(struct Lexer *lx)
{
        for (size_t i = 0; i < lx->tokens->size; i++) {
                struct Token *tok = token_list_get(lx->tokens, i);

                if (!tok) {
                        continue;
                }

                if (tok->type != WHITESPACE && tok->type != COMMENT &&
                    tok->type != MULTILINE_COMMENT) {
                        fprintf(lx->symbol_table_file,
                                "%-30s %-30s Line: %-5d Col: %-5d\n",
                                tok_type_to_str(tok->type),
                                tok->lexeme,
                                tok->line,
                                tok->col);
                }
        }
}