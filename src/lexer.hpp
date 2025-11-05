#include "./dfa.hpp"

class LexerToken
{
public:
  std::string lexeme;
  std::string type;

  LexerToken(std::string lexeme, std::string type) : lexeme(lexeme), type(type)
  {
  }
};

class TableDrivenLexer
{
private:
  TransitionTable transitionTable;
  std::vector<LexerToken> tokens;

public:
  TableDrivenLexer(const TransitionTable &table) : transitionTable(table) {}

  void lex(const std::string &input);

  // calls lex()
  void createSymbolTable(const std::string &input,
                         const std::string &file,
                         bool printWhitespace = false);
};