#ifndef NFA_H
#define NFA_H

#include "./regex_parser.hpp"
#include <memory>
#include <optional>
#include <set>

class NFAState
{
public:
  int id;
  bool isAccept;
  std::optional<std::string> tokenType; // for lexer use
  int tokenPriority = INT_MAX; // lower = higher priority

  NFAState(int id) : id(id), isAccept(false) {}
};

// represent e-moves in NFATransition
const char EPSILON = '\0';

struct NFATransition {
  NFAState *from;
  NFAState *to;
  char symbol; // use EPSILON as symbol for e-move
  std::set<char> charClass; // for char class regex
  bool isCharClass;

  NFATransition(NFAState *f, NFAState *t, char sym)
      : from(f), to(t), symbol(sym), isCharClass(false)
  {
  }

  NFATransition(NFAState *f, NFAState *t, const std::set<char> &cc)
      : from(f), to(t), charClass(cc), isCharClass(true)
  {
  }

  bool matches(char c) const;
};

// used for construction only, holds only the start and accept state to
// link intermediate nfas
struct NFAFragment {
  NFAState *start;
  NFAState *accept;

  NFAFragment(NFAState *s, NFAState *a) : start(s), accept(a) {}
};

class NFA
{
private:
  std::vector<std::unique_ptr<NFAState>> states;
  std::vector<NFATransition> transitions;
  int nextStateId; // used when added new states

public:
  NFAState *startState;
  NFAState *acceptState;

  NFA() : nextStateId(0), startState(nullptr), acceptState(nullptr) {}

  NFAState *createState();

  void addTransition(NFAState *from, NFAState *to, char symb);
  void addTransition(NFAState *from,
                     NFAState *to,
                     const std::set<char> &cc); // char class transitions

  // get transitions from specific state
  std::vector<NFATransition *> getTransitions(NFAState *state);

  // get epsilon closure of set of states
  std::set<NFAState *> epsilonClosure(const std::set<NFAState *> &states);

  // simulate move on set of states given input symbol
  std::set<NFAState *> move(const std::set<NFAState *> &states, char symbol);

  // get alphabet (excluding e)
  std::set<char> getAlphabet();

  // simulate nfa on string
  bool matches(const std::string &input);

  void print();
};

class ThompsonConstruction
{
private:
  NFA &nfa;

public:
  ThompsonConstruction(NFA &n) : nfa(n) {}

  NFAFragment build(const std::shared_ptr<ASTNode> &node);

private:
  NFAFragment buildChar(char c);
  NFAFragment buildCharClass(const std::set<char> &cc);
  NFAFragment buildConcat(NFAFragment left, NFAFragment right);
  NFAFragment buildUnion(NFAFragment left, NFAFragment right);
  NFAFragment buildStar(NFAFragment inner);
  NFAFragment buildPlus(NFAFragment inner);
  NFAFragment buildQuestion(NFAFragment inner);
};

#endif