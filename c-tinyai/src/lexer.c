#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"

void lexer_init(struct Lexer *lexer, char *source)
{
  lexer->source_code = source;
  lexer->position = 0;

  lexer->symbol_table_file = fopen("symbol_table.txt", "w");
  if (!lexer->symbol_table_file) {
    perror("Failed to open symbol table file");
    exit(EXIT_FAILURE);
  }
}

void lex(struct Lexer *lexer)
{
  fprintf(lexer->symbol_table_file, "%-30s %s\n", "Lexeme", "Token Type");

  int start_state = START_STATE_ID;
  size_t source_len = strlen(lexer->source_code);
  size_t current_pos = (size_t)lexer->position;

  while (current_pos < source_len) {
    int current_state = start_state;
    int last_accepting_state = -1;
    int last_accepting_pos = -1;

    size_t pos = current_pos;
    while (pos < source_len) {
      unsigned char uc = (unsigned char)lexer->source_code[pos];
      int next_state = TRANSITION_TABLE[current_state][uc];

      if (next_state == -1) {
        break;
      }

      current_state = next_state;

      if (ACCEPT_STATE_IDS[current_state]) {
        last_accepting_state = current_state;
        last_accepting_pos = (int)pos;
      }

      pos++;
    }

    if (last_accepting_state != -1) {
      size_t lexeme_length = (size_t)last_accepting_pos - current_pos + 1;
      char *lexeme = (char *)malloc(lexeme_length + 1);
      memcpy(lexeme, &lexer->source_code[current_pos], lexeme_length);
      lexeme[lexeme_length] = '\0';

      TokenType token_type = STATE_TOKEN_TYPE[last_accepting_state];

      struct Token token;
      token.type = token_type;
      token.lexeme = lexeme;

      if (token.type != WHITESPACE) {
        fprintf(lexer->symbol_table_file,
                "%-30s %s\n",
                token.lexeme,
                tok_type_to_str(token.type));
        // fprintf(
        // stdout, "%-30s %s\n", token.lexeme, tok_type_to_str(token.type));
      }

      free(lexeme);

      current_pos = (size_t)last_accepting_pos + 1;
      lexer->position = (int)current_pos;
    } else {
      fprintf(lexer->symbol_table_file,
              "%-30c %s\n",
              lexer->source_code[current_pos],
              "UNKNOWN");
      // fprintf(stdout, "%-30c %s\n", lexer->source_code[current_pos],
      // "UNKNOWN");
      current_pos++;
      lexer->position = (int)current_pos;
    }
  }
}