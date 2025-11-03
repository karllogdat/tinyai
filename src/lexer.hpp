#include "./dfa.hpp"

class TableDrivenLexer
{
private:
  TransitionTable transitionTable;

public:
  TableDrivenLexer(const TransitionTable &table) : transitionTable(table) {}

  void lex(const std::string &input);
};