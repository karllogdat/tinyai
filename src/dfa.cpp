#include <iostream>
#include <queue>
#include <stdexcept>
#include "./dfa.hpp"

DFAState *DFA::createState(const std::set<NFAState *> &nfaStates)
{
  int id = states.size();
  auto state = std::make_unique<DFAState>(id, nfaStates);
  DFAState *ptr = state.get();
  states.push_back(std::move(state));
  return ptr;
}

void DFA::addTransition(DFAState *f, DFAState *t, char symb)
{
  transitions.emplace_back(f, t, symb);
}

std::vector<DFATransition *> DFA::getTransitions(DFAState *state)
{
  std::vector<DFATransition *> result;
  for (auto &trans : transitions) {
    if (trans.from == state)
      result.push_back(&trans);
  }
  return result;
}

DFAState *DFA::getNextState(DFAState *state, char symb)
{
  for (auto &trans : transitions) {
    if (trans.from == state && trans.symbol == symb)
      return trans.to;
  }
  return nullptr;
}

MatchResult DFA::matches(const std::string &input)
{
  DFAState *currentState = startState;

  for (char c : input) {
    currentState = getNextState(currentState, c);
    if (!currentState) {
      return { false, std::nullopt };
    }
  }

  return { currentState->isAccept,
           currentState->isAccept ? currentState->tokenType : std::nullopt };
}

DFA SubsetConstruction::convert()
{
  std::cout << "Starting subset construction..." << std::endl;

  if (!nfa.startState) {
    throw std::runtime_error("NFA has no start states");
  }

  std::set<char> alphabet = nfa.getAlphabet();
  std::cout << "Alphabet size: " << alphabet.size() << std::endl;

  std::set<NFAState *> nfaStartStates = { nfa.startState };
  std::set<NFAState *> startClosure = nfa.epsilonClosure(nfaStartStates);

  DFAState *dfaStartState = dfa.createState(startClosure);
  dfa.startState = dfaStartState;
  stateMapping[startClosure] = dfaStartState;

  // Check for accept states and token types in the closure
  for (NFAState *nfaState : startClosure) {
    if (nfaState->isAccept) {
      dfaStartState->isAccept = true;
      if (nfaState->tokenType) {
        dfaStartState->tokenType = nfaState->tokenType;
        std::cout << "Setting token type for DFA state " << dfaStartState->id
                  << ": " << *nfaState->tokenType << std::endl;
      }
      dfa.acceptStates.insert(dfaStartState);
    }
  }

  std::queue<std::set<NFAState *>> workQueue;
  workQueue.push(startClosure);
  std::set<std::set<NFAState *>> processed;

  while (!workQueue.empty()) {
    std::set<NFAState *> currentNFAStates = workQueue.front();
    workQueue.pop();

    if (processed.find(currentNFAStates) != processed.end())
      continue;
    processed.insert(currentNFAStates);

    DFAState *currentDFAState = stateMapping[currentNFAStates];

    for (char symbol : alphabet) {
      std::set<NFAState *> nextStates = nfa.move(currentNFAStates, symbol);
      nextStates = nfa.epsilonClosure(nextStates);

      if (nextStates.empty())
        continue;

      DFAState *nextDFAState;
      if (stateMapping.find(nextStates) == stateMapping.end()) {
        nextDFAState = dfa.createState(nextStates);
        stateMapping[nextStates] = nextDFAState;
        workQueue.push(nextStates);

        // Check for accept states and token types in the next states
        for (NFAState *nfaState : nextStates) {
          if (nfaState->isAccept) {
            nextDFAState->isAccept = true;
            if (nfaState->tokenType) {
              nextDFAState->tokenType = nfaState->tokenType;
              std::cout << "Setting token type for DFA state "
                        << nextDFAState->id << ": " << *nfaState->tokenType
                        << std::endl;
            }
            dfa.acceptStates.insert(nextDFAState);
          }
        }
      } else {
        nextDFAState = stateMapping[nextStates];
      }

      dfa.addTransition(currentDFAState, nextDFAState, symbol);
    }
  }

  std::cout << "DFA construction complete with " << dfa.states.size()
            << " states" << std::endl;
  return std::move(dfa);
}

DFA SubsetConstruction::getDFA() { return std::move(dfa); }

MatchResult TransitionTable::matches(const std::string &str)
{
  if (startStateId == -1)
    return { false, std::nullopt };

  int current = startStateId;

  for (char c : str) {
    if (symbolToId.find(c) == symbolToId.end())
      return { false, std::nullopt };

    int symbolId = symbolToId[c];
    int next = table[current][symbolId];

    if (next == -1)
      return { false, std::nullopt };

    current = next;
  }

  if (acceptStateIds.find(current) != acceptStateIds.end()) {
    return { true, stateTokenTypes[current] };
  }
  return { false, std::nullopt };
}

TransitionTable TransitionTableBuilder::build()
{
  TransitionTable table;

  if (!dfa.startState) {
    throw std::runtime_error("DFA has no start state");
  }

  std::set<char> alphabet;
  for (const auto &trans : dfa.transitions) {
    alphabet.insert(trans.symbol);
  }
  table.alphabet = std::vector<char>(alphabet.begin(), alphabet.end());

  for (size_t i = 0; i < table.alphabet.size(); i++) {
    table.symbolToId[table.alphabet[i]] = i;
  }

  size_t statesCount = dfa.states.size();
  size_t symbolCount = table.alphabet.size();
  table.table.resize(statesCount, std::vector<int>(symbolCount, -1));

  for (const auto &trans : dfa.transitions) {
    int from = trans.from->id;
    int to = trans.to->id;
    int symb = table.symbolToId[trans.symbol];

    table.table[from][symb] = to;
  }

  table.startStateId = dfa.startState->id;

  for (const auto &state : dfa.acceptStates) {
    table.acceptStateIds.insert(state->id);
    if (state->tokenType) {
      table.stateTokenTypes[state->id] = *(state->tokenType);
    }
  }

  return table;
}