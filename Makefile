# Makefile

CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -I./src
SRC = src/main.cpp src/regex_parser.cpp src/nfa.cpp src/dfa.cpp src/engine.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = lexer

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean