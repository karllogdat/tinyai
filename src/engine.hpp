#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>
#include <vector>
#include <memory>
#include "./nfa.hpp"
#include "./dfa.hpp"
#include "./regex_parser.hpp"

class RegexEngine {
private:
    std::vector<std::string> regexPatterns;
    NFA combinedNFA;
    DFA dfa;

public:
    RegexEngine() = default;

    void addRegex(const std::string &regex);
    void combineRegexes();
    void convertToDFA();
    bool matches(const std::string &input);
    void printCombinedNFA();
};

#endif