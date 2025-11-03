#include "./lexer.hpp"
#include <iostream>

void TableDrivenLexer::lex(const std::string &input)
{
  int startState = transitionTable.startStateId;
  int currentPos = 0;
  int tokenStart = 0; // Track where current token started

  while (currentPos <= input.size()) { // Changed to <= to handle last token
    int currentState = startState;
    int lastAcceptState = -1;
    int lastAcceptPos = -1;

    // Try to match longest possible token from current position
    int pos = tokenStart;
    while (pos < input.size()) {
      char c = input[pos];

      // Check if character is in alphabet
      auto symbolIt = transitionTable.symbolToId.find(c);
      if (symbolIt == transitionTable.symbolToId.end()) {
        break;
      }

      int nextState = transitionTable.table[currentState][symbolIt->second];
      if (nextState == -1) {
        break;
      }

      currentState = nextState;
      if (transitionTable.acceptStateIds.count(currentState)) {
        lastAcceptState = currentState;
        lastAcceptPos = pos;
      }
      pos++;
    }

    // Emit token if we found an accepting state
    if (lastAcceptState != -1) {
      int lexemeLength = lastAcceptPos - tokenStart + 1;
      std::string lexeme = input.substr(tokenStart, lexemeLength);
      std::string tokenType = transitionTable.stateTokenTypes[lastAcceptState];

      if (tokenType != "WHITESPACE") { // Skip whitespace tokens
        std::cout << "Token: " << tokenType << ", Lexeme: \"" << lexeme << "\""
                  << std::endl;
      }

      tokenStart = lastAcceptPos + 1;
      currentPos = tokenStart;
    } else {
      // No valid token found - skip one character
      if (tokenStart < input.size()) {
        std::cerr << "Invalid input at position " << tokenStart << ": '"
                  << input[tokenStart] << "'" << std::endl;
        tokenStart++;
        currentPos = tokenStart;
      } else {
        break;
      }
    }
  }
}