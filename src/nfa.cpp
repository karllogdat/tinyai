#include "./nfa.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>

bool NFATransition::matches(char c) const
{
  if (isCharClass)
    return charClass.find(c) != charClass.end();

  return symbol == c || symbol == EPSILON;
}

NFAState *NFA::createState()
{
  auto state = std::make_unique<NFAState>(nextStateId++);
  NFAState *ptr = state.get();
  states.push_back(std::move(state));
  return ptr;
}

void NFA::addTransition(NFAState *from, NFAState *to, char symb)
{
  // Add similar debug output for character class transitions
  std::cout << "Adding char transition from state " << from->id << " to state "
            << to->id << std::endl;

  // Verify both states exist
  bool fromExists = false;
  bool toExists = false;
  for (const auto &state : states) {
    if (state.get() == from)
      fromExists = true;
    if (state.get() == to)
      toExists = true;
  }

  if (!fromExists || !toExists) {
    std::cout << "Error: Attempting to add transition with non-existent state!"
              << std::endl;
    std::cout << "FromState exists: " << fromExists << std::endl;
    std::cout << "ToState exists: " << toExists << std::endl;
    throw std::runtime_error("Invalid state in addTransition");
  }
  transitions.emplace_back(from, to, symb);
}

void NFA::addTransition(NFAState *from, NFAState *to, const std::set<char> &cc)
{
  // Add similar debug output for character class transitions
  std::cout << "Adding char class transition from state " << from->id
            << " to state " << to->id << std::endl;

  // Verify both states exist
  bool fromExists = false;
  bool toExists = false;
  for (const auto &state : states) {
    if (state.get() == from)
      fromExists = true;
    if (state.get() == to)
      toExists = true;
  }

  if (!fromExists || !toExists) {
    std::cout << "Error: Attempting to add transition with non-existent state!"
              << std::endl;
    std::cout << "FromState exists: " << fromExists << std::endl;
    std::cout << "ToState exists: " << toExists << std::endl;
    throw std::runtime_error("Invalid state in addTransition");
  }
  transitions.emplace_back(from, to, cc);
}

std::vector<NFATransition *> NFA::getTransitions(NFAState *state)
{
  std::vector<NFATransition *> result;
  for (auto &trans : transitions) {
    if (trans.from == state) {
      result.push_back(&trans);
    }
  }
  return result;
}

std::set<NFAState *> NFA::epsilonClosure(const std::set<NFAState *> &states)
{
  std::set<NFAState *> closure = states;
  std::vector<NFAState *> stack(states.begin(), states.end());

  while (!stack.empty()) {
    NFAState *state = stack.back();
    stack.pop_back();

    for (auto &trans : transitions) {
      if (trans.from == state && trans.symbol == EPSILON &&
          !trans.isCharClass) {
        if (closure.find(trans.to) == closure.end()) {
          closure.insert(trans.to);
          stack.push_back(trans.to);
        }
      }
    }
  }

  return closure;
}

std::set<NFAState *> NFA::move(const std::set<NFAState *> &states, char symbol)
{
  std::set<NFAState *> result;

  for (NFAState *state : states) {
    for (auto &trans : transitions) {
      if (trans.from == state) {
        if (trans.isCharClass &&
            trans.charClass.find(symbol) != trans.charClass.end()) {
          result.insert(trans.to);
        } else if (!trans.isCharClass && trans.symbol == symbol) {
          result.insert(trans.to);
        }
      }
    }
  }

  return result;
}

std::set<char> NFA::getAlphabet()
{
  std::set<char> alphabet;

  for (const auto &trans : transitions) {
    if (trans.isCharClass)
      alphabet.insert(trans.charClass.begin(), trans.charClass.end());
    else if (trans.symbol != EPSILON)
      alphabet.insert(trans.symbol);
  }

  return alphabet;
}

bool NFA::matches(const std::string &input)
{
  std::set<NFAState *> currentStates = { startState };
  currentStates = epsilonClosure(currentStates);

  for (char c : input) {
    std::set<NFAState *> nextStates;

    for (NFAState *state : currentStates) {
      for (auto &trans : transitions) {
        if (trans.from == state) {
          if (trans.isCharClass &&
              trans.charClass.find(c) != trans.charClass.end()) {
            nextStates.insert(trans.to);
          } else if (!trans.isCharClass && trans.symbol == c) {
            nextStates.insert(trans.to);
          }
        }
      }
    }

    currentStates = epsilonClosure(nextStates);

    if (currentStates.empty()) {
      return false;
    }
  }

  return currentStates.find(acceptState) != currentStates.end();
}

void NFA::print()
{
  std::cout << "NFA States: " << states.size() << std::endl;
  std::cout << "Start State: " << startState->id << std::endl;
  std::cout << "Accept State: " << acceptState->id << std::endl;
  std::cout << "\nTransitions: " << std::endl;

  for (const auto &trans : transitions) {
    std::cout << "  State " << trans.from->id << "-> State" << trans.to->id;

    if (trans.isCharClass) {
      std::cout << " [char class: ";
      int count = 0;
      for (char c : trans.charClass) {
        if (count++ > 5) {
          std::cout << "...";
          break;
        }
        if (isprint(c))
          std::cout << c;
        else if (c == '\n')
          std::cout << "\\n";
        else if (c == '\t')
          std::cout << "\\t";
        else
          std::cout << "\\" << (int)c;
      }
      std::cout << "]";
    } else if (trans.symbol == EPSILON) {
      std::cout << " [EPSILON]";
    } else {
      std::cout << "['" << trans.symbol << "']";
    }
    std::cout << std::endl;
  }
}

NFAFragment ThompsonConstruction::buildChar(char c)
{
  NFAState *start = nfa.createState();
  NFAState *accept = nfa.createState();
  nfa.addTransition(start, accept, c);
  return NFAFragment(start, accept);
}

NFAFragment ThompsonConstruction::buildCharClass(const std::set<char> &cc)
{
  NFAState *start = nfa.createState();
  NFAState *accept = nfa.createState();
  nfa.addTransition(start, accept, cc);
  return NFAFragment(start, accept);
}

NFAFragment ThompsonConstruction::buildConcat(NFAFragment left,
                                              NFAFragment right)
{
  nfa.addTransition(left.accept, right.start, EPSILON);
  return NFAFragment(left.start, right.accept);
}

NFAFragment ThompsonConstruction::buildUnion(NFAFragment left,
                                             NFAFragment right)
{
  NFAState *start = nfa.createState();
  NFAState *accept = nfa.createState();

  nfa.addTransition(start, left.start, EPSILON);
  nfa.addTransition(start, right.start, EPSILON);

  nfa.addTransition(left.accept, accept, EPSILON);
  nfa.addTransition(right.accept, accept, EPSILON);

  return NFAFragment(start, accept);
}

NFAFragment ThompsonConstruction::buildStar(NFAFragment inner)
{
  NFAState *start = nfa.createState();
  NFAState *accept = nfa.createState();

  nfa.addTransition(start, inner.start, EPSILON);
  nfa.addTransition(start, accept, EPSILON);

  nfa.addTransition(inner.accept, inner.start, EPSILON);
  nfa.addTransition(inner.accept, accept, EPSILON);

  return NFAFragment(start, accept);
}

NFAFragment ThompsonConstruction::buildPlus(NFAFragment inner)
{
  NFAState *start = nfa.createState();
  NFAState *accept = nfa.createState();

  nfa.addTransition(start, inner.start, EPSILON);

  nfa.addTransition(inner.accept, inner.start, EPSILON);
  nfa.addTransition(inner.accept, accept, EPSILON);

  return NFAFragment(start, accept);
}

NFAFragment ThompsonConstruction::buildQuestion(NFAFragment inner)
{
  NFAState *start = nfa.createState();
  NFAState *accept = nfa.createState();

  nfa.addTransition(start, inner.start, EPSILON);
  nfa.addTransition(start, accept, EPSILON);

  nfa.addTransition(inner.accept, accept, EPSILON);

  return NFAFragment(start, accept);
}

NFAFragment ThompsonConstruction::build(const std::shared_ptr<ASTNode> &node)
{
  if (!node) {
    throw std::runtime_error("Null ASTNode");
  }

  NFAFragment left(nullptr, nullptr);
  NFAFragment right(nullptr, nullptr);
  NFAFragment inner(nullptr, nullptr);
  switch (node->type) {
    case NodeType::CHAR:
      return buildChar(node->value);

    case NodeType::CHAR_CLASS:
      return buildCharClass(node->charClass);

    case NodeType::CONCAT:
      left = build(node->left);
      right = build(node->right);
      return buildConcat(left, right);

    case NodeType::UNION:
      left = build(node->left);
      right = build(node->right);
      return buildUnion(left, right);

    case NodeType::STAR:
      inner = build(node->left);
      return buildStar(inner);

    case NodeType::PLUS:
      inner = build(node->left);
      return buildPlus(inner);

    case NodeType::QUESTION:
      inner = build(node->left);
      return buildQuestion(inner);
  }

  throw std::runtime_error("Unknown node type");
}