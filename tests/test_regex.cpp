#include <iostream>
#include <string>
#include <vector>
#include "../src/regex_parser.hpp"
#include "../src/nfa.hpp"
#include "../src/dfa.hpp"

void testRegexCombination(const std::vector<std::string>& patterns, const std::vector<std::string>& testStrings) {
    NFA nfa;
    ThompsonConstruction thompson(nfa);
    
    // Combine regex patterns into a single e-NFA
    NFAFragment combinedFragment(nullptr, nullptr);
    for (const auto& pattern : patterns) {
        RegexLexer lexer(pattern);
        std::vector<Token> tokens = lexer.tokenize();
        RegexParser parser(tokens);
        auto ASTroot = parser.parse();
        auto fragment = thompson.build(ASTroot);
        
        if (combinedFragment.start == nullptr) {
            combinedFragment = fragment; // First fragment
        } else {
            combinedFragment = thompson.buildConcat(combinedFragment, fragment); // Concatenate
        }
    }

    nfa.startState = combinedFragment.start;
    nfa.acceptState = combinedFragment.accept;
    nfa.acceptState->isAccept = true;

    // Convert NFA to DFA
    SubsetConstruction converter(nfa);
    DFA dfa = converter.convert();

    // Test matching strings
    for (const auto& str : testStrings) {
        bool match = dfa.matches(str);
        std::cout << "Testing \"" << str << "\": " << (match ? "MATCH" : "NO MATCH") << std::endl;
    }
}

int main() {
    std::vector<std::string> regexPatterns = {
        "[a-zA-Z_][a-zA-Z0-9_]*", // Matches identifiers
        "\\d+",                   // Matches integers
        "\"[^\"]*\""              // Matches strings
    };

    std::vector<std::string> testStrings = {
        "myVariable", // match
        "12345",      // match
        "\"Hello, World!\"", // match
        "invalid-variable", // reject
        "not_a_string", // reject
        "123abc" // reject
    };

    testRegexCombination(regexPatterns, testStrings);
    return 0;
}