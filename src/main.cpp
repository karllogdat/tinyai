#include "./dfa.hpp"
#include "./nfa.hpp"
#include "./regex_parser.hpp"
#include "./lexer.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

std::string readFileToString(const std::string &filename)
{
  std::ifstream file(filename);
  std::ostringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

int main()
{
  std::cout << "Starting Lexer Engine Test..." << std::endl;
  std::vector<RegexPattern> patterns = {
    RegexPattern("[a-zA-Z][a-zA-Z0-9_]*", "IDENTIFIER"),
    RegexPattern("#[^\n]*",
                 "COMMENT"), // Match everything except newline after #
    RegexPattern("\\\"\\\"\\\"[\\s\\S]*\\\"\\\"\\\"", "MULTILINE_STRING"),
    RegexPattern("\\\"[^\\\"\\n]*\\\"", "STRING"),
    RegexPattern("\\'[^\\'\\n]\\'", "CHAR"),
    RegexPattern("\\d+", "NUMBER"),
    RegexPattern("\\s+", "WHITESPACE"),
    RegexPattern("\\d+\\.\\d+", "FLOAT"),
    RegexPattern("\\+", "PLUS"),
    RegexPattern("\\*", "STAR"),
    RegexPattern("\\*\\*", "POWER"),
    RegexPattern("-", "MINUS"),
    RegexPattern("/", "SLASH"),
    RegexPattern("==", "EQUAL"),
    RegexPattern("=", "ASSIGN"),
    RegexPattern("%", "MODULO"),
    RegexPattern("<=", "LESS_EQUAL"),
    RegexPattern("<", "LESS_THAN"),
    RegexPattern(">=", "GREATER_EQUAL"),
    RegexPattern(">", "GREATER_THAN"),
    RegexPattern("!=", "NOT_EQUAL"),
    RegexPattern("\\(", "LEFT_PARENTHESIS"),
    RegexPattern("\\)", "RIGHT_PARENTHESIS"),
    RegexPattern("\\[", "LEFT_SQUARE_BRACKET"),
    RegexPattern("\\]", "RIGHT_SQUARE_BRACKET"),
    RegexPattern("\\{", "LEFT_CURLY_BRACE"),
    RegexPattern("\\}", "RIGHT_CURLY_BRACE"),
    RegexPattern(",", "COMMA"),
    RegexPattern(":", "COLON"),
    RegexPattern(";", "SEMI_COLON"),
  };

  TransitionTableGenerator tableGenerator(patterns);
  TransitionTable table = tableGenerator.generate();

  TableDrivenLexer lexer(table);
  std::string input = readFileToString("test.ai");
  lexer.lex(input);

  return 0;
}