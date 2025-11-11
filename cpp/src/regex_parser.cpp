#include "./regex_parser.hpp"
#include <cctype>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

// expands character escaped characters: '\n' '\t' etc.
char RegexLexer::expandEscape(char c)
{
  switch (c) {
    case 'n':
      return '\n';
    case 't':
      return '\t';
    case 'r':
      return '\r';
    case 'f':
      return '\f';
    case 'v':
      return '\v';
    case '0':
      return '\0';
    default:
      return c;
  }
}

// expand metacharacters such as \d -> [0-9]. supports '^' (negation)
std::set<char> RegexLexer::getMetaCharClass(char c, bool negate)
{
  std::set<char> chars;

  switch (c) {
    case 'd': // digits
      for (char ch = '0'; ch <= '9'; ch++) {
        chars.insert(ch);
      }
      break;
    case 'w': // [a-zA-Z0-9_]
      for (char ch = 'a'; ch <= 'z'; ch++)
        chars.insert(ch);
      for (char ch = 'A'; ch <= 'Z'; ch++)
        chars.insert(ch);
      for (char ch = '0'; ch <= '9'; ch++)
        chars.insert(ch);
      chars.insert('_');
      break;
    case 's': // whitespace characters
      chars.insert(' ');
      chars.insert('\r');
      chars.insert('\t');
      chars.insert('\n');
      chars.insert('\f');
      chars.insert('\v');
      break;
  }

  if (negate && !chars.empty()) {
    std::set<char> negateChars;
    for (int i = 0; i < 128; i++) {
      if (chars.find(i) == chars.end()) {
        negateChars.insert(i);
      }
    }

    return negateChars;
  }

  return chars;
}

std::vector<Token> RegexLexer::tokenize()
{
  std::vector<Token> tokens;

  while (pos < input.length()) {
    char c = input[pos];

    if (c == '(') {
      tokens.push_back(Token(TokenType::LPAREN, c));
      pos++;
    } else if (c == ')') {
      tokens.push_back(Token(TokenType::RPAREN, c));
      pos++;
    } else if (c == '*') {
      tokens.push_back(Token(TokenType::STAR, c));
      pos++;
    } else if (c == '|') {
      tokens.push_back(Token(TokenType::PIPE, c));
      pos++;
    } else if (c == '+') {
      tokens.push_back(Token(TokenType::PLUS, c));
      pos++;
    } else if (c == '?') {
      tokens.push_back(Token(TokenType::QUESTION, c));
      pos++;
    } else if (c == '[') {
      tokens.push_back(parseCharacterClass());
    } else if (c == '"') {
      // Parse quoted literal sequence: everything inside "..." is taken
      // literally.
      pos++; // consume opening "
      while (pos < input.length() && input[pos] != '"') {
        if (input[pos] == '\\') {
          pos++;
          if (pos < input.length()) {
            // inside quotes, treat escapes as literal characters (\n, \t, etc.)
            tokens.push_back(Token(TokenType::CHAR, expandEscape(input[pos])));
            pos++;
          } else {
            break;
          }
        } else {
          tokens.push_back(Token(TokenType::CHAR, input[pos]));
          pos++;
        }
      }
      // consume closing " if present
      if (pos < input.length() && input[pos] == '"')
        pos++;
    } else if (c == '\\') {
      pos++;
      if (pos < input.length()) {
        char next = input[pos];

        if (next == 'd' || next == 'w' || next == 's') {
          auto charClass = getMetaCharClass(next, false);
          tokens.push_back(Token(TokenType::CHAR_CLASS, charClass));
          pos++;
        } else if (next == 'D' || next == 'W' || next == 'S') {
          auto charClass = getMetaCharClass(tolower(next), true);
          tokens.push_back(Token(TokenType::CHAR_CLASS, charClass));
          pos++;
        } else {
          // regular escape char (not \d etc)
          tokens.push_back(Token(TokenType::CHAR, expandEscape(next)));
          pos++;
        }
      }
    } else {
      tokens.push_back(Token(TokenType::CHAR, c));
      pos++;
    }
  }

  tokens.push_back(Token(TokenType::END));
  return tokens;
}

Token RegexLexer::parseCharacterClass()
{
  pos++; // consume '['
  std::set<char> chars;
  bool negated = false;

  if (pos < input.length() && input[pos] == '^') {
    negated = true;
    pos++;
  }

  while (pos < input.length() && input[pos] != ']') {
    char start = input[pos];

    // handle escaped characters inside charclass
    if (input[pos] == '\\') {
      pos++;
      if (pos < input.length()) {
        char next = input[pos];

        if (next == 'd' || next == 'w' || next == 's') {
          auto metaChars = getMetaCharClass(next, false);
          chars.insert(metaChars.begin(), metaChars.end());
          pos++;
          continue;
        } else if (next == 'D' || next == 'W' || next == 'S') {
          auto metaChars = getMetaCharClass(tolower(next), true);
          chars.insert(metaChars.begin(), metaChars.end());
          pos++;
          continue;
        } else {
          // regular escape char (not \d etc)
          start = expandEscape(next);
          pos++;
        }
      } else {
        break;
      }
    } else {
      pos++;
    }

    if (pos < input.length() && input[pos] == '-' && pos + 1 < input.length() &&
        input[pos] != ']') {
      pos++; // skip '-'

      char end;
      if (input[pos] == '\\') {
        pos++;
        if (pos < input.length()) {
          end = expandEscape(input[pos]);
          pos++;
        } else {
          break;
        }
      } else {
        end = input[pos];
        pos++;
      }

      for (char ch = start; ch <= end; ch++) {
        chars.insert(ch);
      }
    } else {
      chars.insert(start);
    }
  }

  if (pos < input.length() && input[pos] == ']') {
    pos++;
  }

  if (negated) {
    std::set<char> negatedChars;
    for (int c = 0; c < 128; c++) {
      if (chars.find(c) == chars.end()) {
        negatedChars.insert(c);
      }
    }
    chars = negatedChars;
  }

  return Token(TokenType::CHAR_CLASS, chars);
}

std::shared_ptr<ASTNode> RegexParser::parse() { return parseUnion(); }

Token RegexParser::peek() { return tokens[pos]; }

Token RegexParser::consume() { return tokens[pos++]; }

bool RegexParser::isAtom()
{
  TokenType t = peek().type;
  return t == TokenType::CHAR || t == TokenType::LPAREN ||
      t == TokenType::DOT || t == TokenType::CHAR_CLASS;
}

std::shared_ptr<ASTNode> RegexParser::parseUnion()
{
  auto left = parseConcat();

  while (peek().type == TokenType::PIPE) {
    consume(); // consumes '|'
    auto right = parseConcat();
    auto node = std::make_shared<ASTNode>(NodeType::UNION);
    node->left = left;
    node->right = right;
    left = node;
  }

  return left;
}

std::shared_ptr<ASTNode> RegexParser::parseConcat()
{
  auto left = parseClosure();

  while (isAtom()) {
    auto right = parseClosure();
    auto node = std::make_shared<ASTNode>(NodeType::CONCAT);
    node->left = left;
    node->right = right;
    left = node;
  }

  return left;
}

std::shared_ptr<ASTNode> RegexParser::parseClosure()
{
  auto node = parseAtom();

  while (true) {
    TokenType t = peek().type;

    if (t == TokenType::STAR) {
      consume(); // consumes '*'
      auto star = std::make_shared<ASTNode>(NodeType::STAR);
      star->left = node;
      node = star;
    } else if (t == TokenType::PLUS) {
      consume();
      auto plus = std::make_shared<ASTNode>(NodeType::PLUS);
      plus->left = node;
      node = plus;
    } else if (t == TokenType::QUESTION) {
      consume();
      auto qs = std::make_shared<ASTNode>(NodeType::QUESTION);
      qs->left = node;
      node = qs;
    } else {
      break;
    }
  }

  return node;
}

std::shared_ptr<ASTNode> RegexParser::parseAtom()
{
  Token t = peek();

  if (t.type == TokenType::CHAR) {
    consume();
    return std::make_shared<ASTNode>(NodeType::CHAR, t.value);
  } else if (t.type == TokenType::CHAR_CLASS) {
    consume();
    return std::make_shared<ASTNode>(NodeType::CHAR_CLASS, t.charClass);
  } else if (t.type == TokenType::DOT) {
    consume();
    std::set<char> chars;
    for (int c = 0; c < 128; c++) {
      if (c != '\n')
        chars.insert(c);
    }
    return std::make_shared<ASTNode>(NodeType::CHAR_CLASS, chars);
  } else if (t.type == TokenType::LPAREN) {
    consume();
    auto node = parseUnion();
    if (peek().type == TokenType::RPAREN) {
      consume();
    }
    return node;
  }

  throw std::runtime_error("unexpected token");
}

std::string charToString(char c)
{
  switch (c) {
    case '\n':
      return "\\n";
    case '\t':
      return "\\t";
    case '\r':
      return "\\r";
    case '\f':
      return "\\f";
    case '\v':
      return "\\v";
    case '\0':
      return "\\0";
    case ' ':
      return "' '";
    default:
      if (isprint(c))
        return std::string(1, c);
      else
        return "\\" + std::to_string((int)c);
  }
}

void printAST(const std::shared_ptr<ASTNode> &node, int depth)
{
  if (!node)
    return;

  std::string indent(depth * 2, ' ');

  // used in CHAR_CLASS case, breaks the switch if not used
  int count;
  switch (node->type) {
    case NodeType::CHAR:
      std::cout << indent << "CHAR: " << charToString(node->value) << std::endl;
      break;
    case NodeType::CHAR_CLASS:
      std::cout << indent << "CHAR CLASS: [";
      count = 0;
      for (char c : node->charClass) {
        if (count++ > 0)
          std::cout << ", ";
        std::cout << charToString(c);
        if (count > 10) {
          std::cout << "...(" << node->charClass.size() << " total)";
          break;
        }
      }
      std::cout << "]" << std::endl;
      break;
    case NodeType::CONCAT:
      std::cout << indent << "CONCAT" << std::endl;
      printAST(node->left, depth + 1);
      printAST(node->right, depth + 1);
      break;
    case NodeType::UNION:
      std::cout << indent << "UNION" << std::endl;
      printAST(node->left, depth + 1);
      printAST(node->right, depth + 1);
      break;
    case NodeType::STAR:
      std::cout << indent << "STAR" << std::endl;
      printAST(node->left, depth + 1);
      break;
    case NodeType::PLUS:
      std::cout << indent << "PLUS" << std::endl;
      printAST(node->left, depth + 1);
      break;
    case NodeType::QUESTION:
      std::cout << indent << "QUESTION" << std::endl;
      printAST(node->left, depth + 1);
      break;
  }
}