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

  DFAState(int id, const std::set<NFAState *> &states)
      : id(id), nfaStates(states), isAccept(false)
  {
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

  bool matches(const std::string &str);

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

  TransitionTable() : startStateId(-1) {}

  bool matches(const std::string &str);
};

class TransitionTableBuilder
{
private:
  DFA &dfa;

public:
  TransitionTableBuilder(DFA &dfa) : dfa(dfa) {}

  TransitionTable build();
};

#endif