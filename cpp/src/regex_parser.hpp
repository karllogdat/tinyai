#ifndef REGEX_PARSER_HPP
#define REGEX_PARSER_HPP

#include <memory>
#include <set>
#include <string>
#include <vector>

struct RegexPattern {
  std::string pattern;
  std::string tokenType; // for lexer use
  int priority; // lower value = higher priority

  RegexPattern(const std::string &p, const std::string &tt, int prio = 0)
      : pattern(p), tokenType(tt), priority(prio)
  {
  }
};

enum class TokenType {
  CHAR,
  LPAREN,
  RPAREN,
  STAR,
  PLUS,
  QUESTION,
  PIPE,
  CONCAT, // implicit, not actually used
  DOT,
  CHAR_CLASS,
  END,
};

struct Token {
  TokenType type;
  char value;
  std::set<char> charClass;

  Token(TokenType t, char v = '\0') : type(t), value(v) {}
  Token(TokenType t, const std::set<char> &cc) : type(t), charClass(cc) {}
};

enum class NodeType {
  CHAR,
  CHAR_CLASS,
  CONCAT,
  UNION,
  STAR,
  PLUS,
  QUESTION,
};

struct ASTNode {
  NodeType type;
  char value;
  std::set<char> charClass;
  std::shared_ptr<ASTNode> left;
  std::shared_ptr<ASTNode> right;

  ASTNode(NodeType t) : type(t), value('\0') {}
  ASTNode(NodeType t, char v) : type(t), value(v) {}
  ASTNode(NodeType t, const std::set<char> &cc) : type(t), charClass(cc) {}
};

class RegexLexer
{
private:
  std::string input;
  size_t pos;

  char expandEscape(char c);
  std::set<char> getMetaCharClass(char c, bool negate = false);

public:
  RegexLexer(const std::string &regex) : input(regex), pos(0) {}

  std::vector<Token> tokenize();

private:
  Token parseCharacterClass();
};

class RegexParser
{
private:
  std::vector<Token> tokens;
  size_t pos;

public:
  RegexParser(const std::vector<Token> &toks) : tokens(toks), pos(0) {}

  std::shared_ptr<ASTNode> parse();

private:
  Token peek();
  Token consume();
  bool isAtom();

  std::shared_ptr<ASTNode> parseUnion();
  std::shared_ptr<ASTNode> parseConcat();
  std::shared_ptr<ASTNode> parseClosure();
  std::shared_ptr<ASTNode> parseAtom();
};

std::string charToString(char c);
void printAST(const std::shared_ptr<ASTNode> &node, int depth = 0);

#endif