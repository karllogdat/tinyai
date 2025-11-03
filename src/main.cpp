#include "./dfa.hpp"
#include "./nfa.hpp"
#include "./regex_parser.hpp"
#include "./engine.hpp"
#include <iostream>
#include <string>
#include <vector>

int main()
{
  std::cout << "Starting Lexer Engine Test..." << std::endl;
  std::vector<RegexPattern> patterns = {
    RegexPattern("[a-zA-Z][a-zA-Z0-9_]*", "IDENTIFIER"),
    RegexPattern("#[^\n]*",
                 "COMMENT"), // Match everything except newline after #
    RegexPattern("\\\"\\\"\\\"[\\s\\S]*\\\"\\\"\\\"", "MULTILINE_STRING"),
    RegexPattern("\\\"[^\\\"\\n]*\\\"", "STRING"),
    RegexPattern("\\d+", "NUMBER"),
    RegexPattern("\\s+", "WHITESPACE"),
    RegexPattern("\\d+\\.\\d+", "FLOAT"),
    RegexPattern("\\+", "PLUS"),
    RegexPattern("\\*", "STAR"),
    RegexPattern("\\*\\*", "POWER"),
    RegexPattern("-", "MINUS"),
    RegexPattern("/", "SLASH"),
    RegexPattern("==", "EQEQ"),
    RegexPattern("=", "EQUALS")
  };

  TransitionTableGenerator tableGenerator(patterns);
  TransitionTable table = tableGenerator.generate();

  std::cout << "Matching strings: " << std::endl;
  std::vector<std::string> testStrings = {
    "myVariable", // match
    "12345", // match
    "   ", // match
    "myVariable2", // match
    "my_Variable", // match
    "0myVariable", // reject
    "my Variable", // reject
    "123abc", // reject
    "12.34", // match
    ".567", // reject
    "89.", // reject
    "+", // match
    "*", // match
    "**", // match
    "-", // match
    "/", // match
    "==", // match EQEQ
    "=", // match EQUALS
    "!==", // reject
    "===", // reject
    "# this is a comment", // match
    "# comment with newline\nnext line", // reject
    "\"\"\"This is a\nmultiline string\"\"\"", // match
    "\"\"\"Unterminated string, should reject", // reject
    "\"\"\"Simple multiline\"\"\"", // should match
    "\"\"\"Multi\nline\nstring\"\"\"", // should match
    "\"\"\"String with \"quotes\" inside\"\"\"", // should match
    "\"\"\"String with \"\"double quotes\"\" inside\"\"\"", // should match
    "\"regular string\"", // match
    "\"unterminated string, should reject" // reject
  };
  for (const auto &str : testStrings) {
    MatchResult match = table.matches(str);
    std::cout << "Test against: " << str << " -> "
              << (match.matched ? "ACCEPT" : "REJECT")
              << (match.matched ? " (" + *match.tokenType + ")" : "")
              << std::endl;
  }

  return 0;
}