# Makefile

CXX = gcc
CXXFLAGS = -Wall -Wextra -Wshadow -I./src
SRC = src/main.c src/lexer.c src/transition_table.c src/token.c
OBJ = $(SRC:.c=.o)
TARGET = lexer

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean