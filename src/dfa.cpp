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
  std::cout << "Matching string length " << str.length() << ": '";
  for (char c : str) {
    if (c == '\n')
      std::cout << "\\n";
    else if (c == '\r')
      std::cout << "\\r";
    else
      std::cout << c;
  }
  std::cout << "'" << std::endl;

  int current = startStateId;

  for (size_t i = 0; i < str.length(); i++) {
    char c = str[i];
    if (symbolToId.find(c) == symbolToId.end()) {
      std::cout << "Failed at position " << i << ", char '" << c
                << "' not in alphabet" << std::endl;
      return { false, std::nullopt };
    }

    int next = table[current][symbolToId[c]];
    if (next == -1) {
      std::cout << "Failed at position " << i << ", no transition from state "
                << current << " on char '" << c << "'" << std::endl;
      return { false, std::nullopt };
    }
    current = next;
  }

  bool accepts = acceptStateIds.find(current) != acceptStateIds.end();
  std::cout << "Ended in state " << current
            << (accepts ? " (accepting)" : " (non-accepting)") << std::endl;

  return { accepts,
           accepts ? std::optional<std::string>{ stateTokenTypes[current] }
                   : std::nullopt };
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

  std::cout << "Alphabet contents: ";
  for (char c : alphabet) {
    if (c == '"')
      std::cout << "\\\"";
    else if (c == '\n')
      std::cout << "\\n";
    else
      std::cout << c;
  }
  std::cout << std::endl;

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

TransitionTable TransitionTableGenerator::generate()
{
  // Combine NFAs for all regex patterns
  NFA combinedNFA;
  ThompsonConstruction thompson(combinedNFA);

  NFAState *globalStart = combinedNFA.createState();
  NFAState *globalAccept = combinedNFA.createState();
  combinedNFA.startState = globalStart;
  combinedNFA.acceptState = globalAccept;
  globalAccept->isAccept = true;

  for (const auto &pattern : patterns) {
    std::cout << "Processing pattern: " << pattern.pattern << std::endl;

    RegexLexer lexer(pattern.pattern);
    std::vector<Token> tokens = lexer.tokenize();

    std::cout << "Tokens: ";
    for (const auto &tk : tokens) {
      if (tk.type == TokenType::CHAR)
        std::cout << " CHAR('"
                  << (tk.value == '\n' ? "\\n" : std::string(1, tk.value))
                  << "')";
      else if (tk.type == TokenType::CHAR_CLASS)
        std::cout << " CHAR_CLASS";
      else if (tk.type == TokenType::LPAREN)
        std::cout << " LPAREN";
      else if (tk.type == TokenType::RPAREN)
        std::cout << " RPAREN";
      else if (tk.type == TokenType::STAR)
        std::cout << " STAR";
      else if (tk.type == TokenType::PIPE)
        std::cout << " PIPE";
      else if (tk.type == TokenType::PLUS)
        std::cout << " PLUS";
      else if (tk.type == TokenType::QUESTION)
        std::cout << " QUESTION";
      else if (tk.type == TokenType::END)
        std::cout << " END";
    }
    std::cout << std::endl;

    RegexParser parser(tokens);
    auto ASTroot = parser.parse();
    std::cout << "Parsed AST." << std::endl;

    auto fragment = thompson.build(ASTroot);
    std::cout << "Built NFA fragment." << std::endl;

    // Ensure fragment's accept is not left marked as final
    if (fragment.accept) {
      fragment.accept->isAccept = false;
      fragment.accept->tokenType = pattern.tokenType;
      std::cout << "Set fragment accept state ID " << fragment.accept->id
                << " token type to " << pattern.tokenType << std::endl;
    }

    // Connect fragments to globals
    combinedNFA.addTransition(globalStart, fragment.start, '\0');
    combinedNFA.addTransition(fragment.accept, globalAccept, '\0');
  }

  SubsetConstruction subsetConv(combinedNFA);
  DFA dfa = subsetConv.convert();

  TransitionTableBuilder tableBuilder(dfa);
  TransitionTable table = tableBuilder.build();

  return table;
}