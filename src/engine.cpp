#include "./engine.hpp"
#include "./nfa.hpp"
#include "./dfa.hpp"
#include "./regex_parser.hpp"
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

class RegexCombiner
{
public:
  static NFA combineRegexes(const std::vector<std::string> &regexPatterns)
  {
    NFA nfa;
    ThompsonConstruction thompson(nfa);

    for (const auto &pattern : regexPatterns) {
      RegexLexer lexer(pattern);
      std::vector<Token> tokens = lexer.tokenize();
      RegexParser parser(tokens);
      auto astRoot = parser.parse();
      auto fragment = thompson.build(astRoot);

      // Link the NFA fragments together
      if (nfa.startState == nullptr) {
        nfa.startState = fragment.start;
      } else {
        nfa.addTransition(nfa.acceptState, fragment.start, EPSILON);
      }
      nfa.acceptState = fragment.accept;
      nfa.acceptState->isAccept = true;
    }

    return nfa;
  }
};

// int main() {
//     std::vector<std::string> regexPatterns = {
//         "[a-zA-Z_][a-zA-Z0-9_]*",
//         "\\d+",
//         "hello|world"
//     };

//     NFA combinedNFA = RegexCombiner::combineRegexes(regexPatterns);
//     combinedNFA.print();

//     std::cout << "Converting combined NFA to DFA..." << std::endl;
//     SubsetConstruction converter(combinedNFA);
//     DFA dfa = converter.convert();

//     TransitionTableBuilder tableBuilder(dfa);
//     TransitionTable table = tableBuilder.build();

//     std::cout << "Testing matching functionality..." << std::endl;
//     std::vector<std::string> testStrings = {
//         "myVariable", // match
//         "12345",      // match
//         "hello",      // match
//         "world",      // match
//         "invalidVar$", // reject
//         "my Variable"  // reject
//     };

//     for (const auto& str : testStrings) {
//         bool match = table.matches(str);
//         std::cout << "Test against: " << str << " -> "
//                   << (match ? "ACCEPT" : "REJECT") << std::endl;
//     }

//     return 0;
// }