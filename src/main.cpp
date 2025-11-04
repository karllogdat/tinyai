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
    // keywords first to ensure they are matched before identifiers
    RegexPattern("if", "IF_TOK"),
    RegexPattern("else", "ELSE_TOK"),
    RegexPattern("elif", "ELIF_TOK"),
    RegexPattern("for", "FOR_TOK"),
    RegexPattern("while", "WHILE_TOK"),
    RegexPattern("break", "BREAK_TOK"),
    RegexPattern("continue", "CONTINUE_TOK"),
    RegexPattern("return", "RETURN_TOK"),
    RegexPattern("func", "FUNC_TOK"),
    RegexPattern("import", "IMPORT_TOK"),
    RegexPattern("from", "FROM_TOK"),
    RegexPattern("as", "AS_TOK"),
    RegexPattern("print", "PRINT_TOK"),
    RegexPattern("in", "IN_TOK"),
    RegexPattern("use", "USE_TOK"),
    RegexPattern("with", "WITH_TOK"),
    RegexPattern("numpy", "NUMPY_TOK"),
    // reserved words data types
    RegexPattern("int", "INT_TOK"),
    RegexPattern("float", "FLOAT_TOK"),
    RegexPattern("bool", "BOOL_TOK"),
    RegexPattern("char", "CHAR_TOK"),
    RegexPattern("string", "STRING_TOK"),
    RegexPattern("tensor", "TENSOR_TOK"),
    RegexPattern("matrix", "MATRIX_TOK"),
    RegexPattern("array", "ARRAY_TOK"),
    RegexPattern("void", "VOID_TOK"),
    // reserved words math functions
    RegexPattern("rand", "RAND_TOK"),
    RegexPattern("zeros", "ZEROS_TOK"),
    RegexPattern("ones", "ONES_TOK"),
    RegexPattern("mean", "MEAN_TOK"),
    RegexPattern("sum", "SUM_TOK"),
    RegexPattern("dot", "DOT_TOK"),
    RegexPattern("max", "MAX_TOK"),
    RegexPattern("min", "MIN_TOK"),
    RegexPattern("std", "STD_TOK"),
    RegexPattern("var", "VAR_TOK"),
    // reserved words data handling
    RegexPattern("to_array", "TOARRAY_TOK"),
    RegexPattern("read_csv", "READCSV_TOK"),
    RegexPattern("to_tensor", "TOTENSOR_TOK"),
    // reserved words statistical operations
    RegexPattern("normalize", "NORMALIZE_TOK"),
    RegexPattern("flatten", "FLATTEN_TOK"),
    RegexPattern("concat", "CONCAT_TOK"),
    RegexPattern("slice", "SLICE_TOK"),
    RegexPattern("sort", "SORT_TOK"),
    RegexPattern("filter", "FILTER_TOK"),
    // noise words
    RegexPattern("then", "THEN"),
    RegexPattern("end", "END"),
    RegexPattern("do", "DO"),
    // general patterns
    RegexPattern("#[^\n]*", "COMMENT"),
    RegexPattern("\\\"\\\"\\\"[\\s\\S]*\\\"\\\"\\\"", "MULTILINE_STRING"),
    RegexPattern("\\\"[^\\\"\\n]*\\\"", "STRING_LITERAL"),
    RegexPattern("\\'[^\\'\\n]\\'", "CHAR_LITERAL"),
    RegexPattern("\\d+", "INT_LITERAL"),
    RegexPattern("\\s+", "WHITESPACE"),
    RegexPattern("\\d+\\.\\d+", "FLOAT_LITERAL"),
    // arithmetic operations
    RegexPattern("\\+", "PLUS"),
    RegexPattern("\\*", "STAR"),
    RegexPattern("\\*\\*", "POWER"),
    RegexPattern("-", "MINUS"),
    RegexPattern("/", "SLASH"),
    // assignment and comparison
    RegexPattern("==", "EQUAL"),
    RegexPattern("=", "ASSIGN"),
    RegexPattern("%", "MODULO"),
    RegexPattern("<=", "LESS_EQUAL"),
    RegexPattern("<", "LESS_THAN"),
    RegexPattern(">=", "GREATER_EQUAL"),
    RegexPattern(">", "GREATER_THAN"),
    RegexPattern("!=", "NOT_EQUAL"),
    // logical operations
    RegexPattern("and", "AND"),
    RegexPattern("or", "OR"),
    RegexPattern("not", "NOT"),
    // delimiters
    RegexPattern("\\(", "LEFT_PARENTHESIS"),
    RegexPattern("\\)", "RIGHT_PARENTHESIS"),
    RegexPattern("\\[", "LEFT_SQUARE_BRACKET"),
    RegexPattern("\\]", "RIGHT_SQUARE_BRACKET"),
    RegexPattern("\\{", "LEFT_CURLY_BRACE"),
    RegexPattern("\\}", "RIGHT_CURLY_BRACE"),
    RegexPattern(",", "COMMA"),
    RegexPattern(":", "COLON"),
    RegexPattern(";", "SEMI_COLON"),
    // identifers (last to ensure keywords are matched first)
    RegexPattern("[a-zA-Z][a-zA-Z0-9_]*", "IDENTIFIER"),
  };

  TransitionTableGenerator tableGenerator(patterns);
  TransitionTable table = tableGenerator.generate();
  tableGenerator.generateToFile("transition_table");

  TableDrivenLexer lexer(table);
  std::string input = readFileToString(inputFile);
  lexer.lex(input);

  return 0;
}