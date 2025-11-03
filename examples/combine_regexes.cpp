#include "./regex_parser.hpp"
#include "./nfa.hpp"
#include "./dfa.hpp"
#include <iostream>
#include <string>
#include <vector>

int main() {
    // Define multiple regex patterns
    std::vector<std::string> regexPatterns = {
        "[a-zA-Z_][a-zA-Z0-9_]*", // Identifiers
        "\\d+",                   // Digits
        "\\s+",                   // Whitespace
    };

    NFA combinedNFA;

    // Combine each regex into a single e-NFA
    ThompsonConstruction thompson(combinedNFA);
    for (const auto &pattern : regexPatterns) {
        RegexLexer lexer(pattern);
        std::vector<Token> tokens = lexer.tokenize();
        RegexParser parser(tokens);
        auto ASTroot = parser.parse();
        NFAFragment fragment = thompson.build(ASTroot);
        
        // Link the NFA fragments together
        if (combinedNFA.startState == nullptr) {
            combinedNFA.startState = fragment.start;
        }
        if (combinedNFA.acceptState != nullptr) {
            combinedNFA.addTransition(combinedNFA.acceptState, fragment.start, EPSILON);
        }
        combinedNFA.acceptState = fragment.accept;
        combinedNFA.acceptState->isAccept = true;
    }

    std::cout << "Combined NFA for patterns: " << std::endl;
    combinedNFA.print();

    // Convert the combined NFA to DFA
    std::cout << "Converting combined NFA to DFA..." << std::endl;
    SubsetConstruction converter(combinedNFA);
    DFA dfa = converter.convert();

    // Build the transition table for the DFA
    TransitionTableBuilder tableBuilder(dfa);
    TransitionTable table = tableBuilder.build();

    // Test matching against various input strings
    std::cout << "Matching strings: " << std::endl;
    std::vector<std::string> testStrings = {
        "myVariable", // match
        "12345",      // match
        "   ",        // match
        "invalidVar$", // reject
        "my Variable", // reject
    };
    for (const auto &str : testStrings) {
        bool match = table.matches(str);
        std::cout << "Test against: " << str << " -> "
                  << (match ? "ACCEPT" : "REJECT") << std::endl;
    }

    return 0;
}