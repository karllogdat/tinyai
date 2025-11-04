#ifndef DFA_H
#define DFA_H

#include <map>
#include <set>
#include "./nfa.hpp"

class DFAState
{
public:
  int id;
  std::set<NFAState *> nfaStates;
  bool isAccept;
  std::optional<std::string> tokenType; // for lexer use

  DFAState(int id, const std::set<NFAState *> &states)
      : id(id), nfaStates(states), isAccept(false)
  {
    for (NFAState *nfaState : states) {
      if (nfaState->tokenType) {
        tokenType = nfaState->tokenType;
        break;
      }
    }
  }
};

class DFATransition
{
public:
  DFAState *from;
  DFAState *to;
  char symbol;

  DFATransition(DFAState *f, DFAState *t, char symb)
      : from(f), to(t), symbol(symb)
  {
  }
};

struct MatchResult {
  bool matched;
  std::optional<std::string> tokenType; // for lexer use

  MatchResult(bool m, const std::optional<std::string> &tt)
      : matched(m), tokenType(tt)
  {
  }
};

class DFA
{
public:
  std::vector<std::unique_ptr<DFAState>> states;
  std::vector<DFATransition> transitions;
  DFAState *startState;
  std::set<DFAState *> acceptStates;

  DFA() : startState(nullptr) {}

  DFAState *createState(const std::set<NFAState *> &nfaStates);

  void addTransition(DFAState *f, DFAState *t, char symb);

  std::vector<DFATransition *> getTransitions(DFAState *state);

  DFAState *getNextState(DFAState *state, char symb);

  MatchResult matches(const std::string &str);

  void print();
};

class SubsetConstruction
{
public:
  NFA &nfa;
  DFA dfa;
  std::map<std::set<NFAState *>, DFAState *> stateMapping;

  SubsetConstruction(NFA &n) : nfa(n) {}

  DFA convert();
  DFA getDFA();
};

class TransitionTable
{
public:
  std::vector<std::vector<int>> table;
  std::vector<char> alphabet;
  std::map<char, int> symbolToId;
  int startStateId;
  std::set<int> acceptStateIds;
  std::map<int, std::string> stateTokenTypes;

  TransitionTable() : startStateId(-1) {}

  MatchResult matches(const std::string &str);
};

class TransitionTableBuilder
{
private:
  DFA &dfa;

public:
  TransitionTableBuilder(DFA &dfa) : dfa(dfa) {}

  TransitionTable build();
};

class TransitionTableGenerator
{
private:
  std::vector<RegexPattern> patterns;

public:
  TransitionTableGenerator(const std::vector<RegexPattern> &pats)
      : patterns(pats)
  {
  }

  TransitionTable generate();
  void generateToFile(const std::string &filename);
};

#endif