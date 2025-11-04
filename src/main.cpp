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

bool fileExists(const std::string &filename)
{
  std::ifstream file(filename);
  return file.good();
}

bool correctExtension(const std::string &filename, const std::string &extension)
{
  size_t extPos = filename.rfind(extension);
  if (extPos == std::string::npos) {
    return false;
  }
  std::string fileExt = filename.substr(extPos);
  return fileExt == extension;
}

std::string readFileToString(const std::string &filename)
{
  std::ifstream file(filename);
  std::ostringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
    return 1;
  }
  std::string inputFile = argv[1];

  if (!fileExists(inputFile)) {
    std::cerr << "Error: File '" << inputFile << "' does not exist."
              << std::endl;
    return 1;
  }

  if (!correctExtension(inputFile, ".ai")) {
    std::cerr << "Error: File '" << inputFile
              << "' does not have the correct '.ai' extension." << std::endl;
    return 1;
  }

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
  tableGenerator.generateToFile("transition_table.h");

  TableDrivenLexer lexer(table);
  std::string input = readFileToString(inputFile);
  lexer.lex(input);

  return 0;
}