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

  std::vector<RegexPattern> patterns = {
    // Keywords (0-19)
    RegexPattern("if", "IF_TOK", 0),
    RegexPattern("else", "ELSE_TOK", 1),
    RegexPattern("elif", "ELIF_TOK", 2),
    RegexPattern("for", "FOR_TOK", 3),
    RegexPattern("while", "WHILE_TOK", 4),
    RegexPattern("break", "BREAK_TOK", 5),
    RegexPattern("continue", "CONTINUE_TOK", 6),
    RegexPattern("return", "RETURN_TOK", 7),
    RegexPattern("func", "FUNC_TOK", 8),
    RegexPattern("import", "IMPORT_TOK", 9),
    RegexPattern("from", "FROM_TOK", 10),
    RegexPattern("as", "AS_TOK", 11),
    RegexPattern("print", "PRINT_TOK", 12),
    RegexPattern("in", "IN_TOK", 13),
    RegexPattern("use", "USE_TOK", 14),
    RegexPattern("with", "WITH_TOK", 15),
    RegexPattern("numpy", "NUMPY_TOK", 16),
    RegexPattern("then", "THEN", 17),
    RegexPattern("end", "END", 18),
    RegexPattern("do", "DO", 19),

    // Data types (20-29)
    RegexPattern("int", "INT_TOK", 20),
    RegexPattern("float", "FLOAT_TOK", 21),
    RegexPattern("bool", "BOOL_TOK", 22),
    RegexPattern("char", "CHAR_TOK", 23),
    RegexPattern("string", "STRING_TOK", 24),
    RegexPattern("tensor", "TENSOR_TOK", 25),
    RegexPattern("matrix", "MATRIX_TOK", 26),
    RegexPattern("array", "ARRAY_TOK", 27),
    RegexPattern("void", "VOID_TOK", 28),

    // Math functions (30-49)
    RegexPattern("rand", "RAND_TOK", 30),
    RegexPattern("zeros", "ZEROS_TOK", 31),
    RegexPattern("ones", "ONES_TOK", 32),
    RegexPattern("mean", "MEAN_TOK", 33),
    RegexPattern("sum", "SUM_TOK", 34),
    RegexPattern("dot", "DOT_TOK", 35),
    RegexPattern("max", "MAX_TOK", 36),
    RegexPattern("min", "MIN_TOK", 37),
    RegexPattern("std", "STD_TOK", 38),
    RegexPattern("var", "VAR_TOK", 39),

    // Data handling functions (50-69)
    RegexPattern("to_array", "TOARRAY_TOK", 50),
    RegexPattern("read_csv", "READCSV_TOK", 51),
    RegexPattern("to_tensor", "TOTENSOR_TOK", 52),
    RegexPattern("normalize", "NORMALIZE_TOK", 53),
    RegexPattern("flatten", "FLATTEN_TOK", 54),
    RegexPattern("concat", "CONCAT_TOK", 55),
    RegexPattern("slice", "SLICE_TOK", 56),
    RegexPattern("sort", "SORT_TOK", 57),
    RegexPattern("filter", "FILTER_TOK", 58),

    // Multi-character operators (70-89)
    RegexPattern("\\*\\*", "POWER", 70),
    RegexPattern("==", "EQUAL", 71),
    RegexPattern("<=", "LESS_EQUAL", 72),
    RegexPattern(">=", "GREATER_EQUAL", 73),
    RegexPattern("!=", "NOT_EQUAL", 74),
    RegexPattern("and", "AND", 75),
    RegexPattern("or", "OR", 76),
    RegexPattern("not", "NOT", 77),

    // Single-character operators (90-109)
    RegexPattern("\\+", "PLUS", 90),
    RegexPattern("\\*", "STAR", 91),
    RegexPattern("-", "MINUS", 92),
    RegexPattern("/", "SLASH", 93),
    RegexPattern("=", "ASSIGN", 94),
    RegexPattern("%", "MODULO", 95),
    RegexPattern("<", "LESS_THAN", 96),
    RegexPattern(">", "GREATER_THAN", 97),

    // Delimiters (110-129)
    RegexPattern("\\(", "LEFT_PARENTHESIS", 110),
    RegexPattern("\\)", "RIGHT_PARENTHESIS", 111),
    RegexPattern("\\[", "LEFT_SQUARE_BRACKET", 112),
    RegexPattern("\\]", "RIGHT_SQUARE_BRACKET", 113),
    RegexPattern("\\{", "LEFT_CURLY_BRACE", 114),
    RegexPattern("\\}", "RIGHT_CURLY_BRACE", 115),
    RegexPattern(",", "COMMA", 116),
    RegexPattern(":", "COLON", 117),
    RegexPattern(";", "SEMI_COLON", 118),

    // Literals (130-149)
    RegexPattern("\\\"\\\"\\\"[\\s\\S]*\\\"\\\"\\\"", "MULTILINE_STRING", 130),
    RegexPattern("\\\"[^\\\"\\n]*\\\"", "STRING_LITERAL", 131),
    RegexPattern("\\'[^\\'\\n]\\'", "CHAR_LITERAL", 132),
    RegexPattern("\\d+\\.\\d+", "FLOAT_LITERAL", 133),
    RegexPattern("\\d+", "INT_LITERAL", 134),
    RegexPattern("true|false", "BOOL_LITERAL", 135),

    // Comments and whitespace (150-159)
    RegexPattern("#[^\n]*", "COMMENT", 150),
    RegexPattern("\\s+", "WHITESPACE", 151),

    // Identifier must be last
    RegexPattern("[a-zA-Z][a-zA-Z0-9_]*", "IDENTIFIER", INT_MAX)
  };

  TransitionTableGenerator tableGenerator(patterns);
  TransitionTable table = tableGenerator.generate();
  tableGenerator.generateToFile("transition_table");

  TableDrivenLexer lexer(table);
  std::string input = readFileToString(inputFile);
  lexer.lex(input);

  return 0;
}