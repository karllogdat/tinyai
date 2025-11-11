# TinyAI

A programming language implementation with lexical analysis capabilities. This repository contains two implementations of TinyAI: one in C and one in C++.

## Overview

TinyAI is a compiler/interpreter project that implements lexical analysis using finite automata theory. Both implementations use a table-driven lexer approach that combines multiple regular expressions into a single epsilon-NFA (e-NFA), converts it to a deterministic finite automaton (DFA), and performs lexical analysis on input files.

## Implementations

### C Implementation (`c/`)

A C implementation of the TinyAI lexer featuring a transition table-driven approach for lexical analysis.

**Building:**

```shell
cd c
make
```

### C++ Implementation (`cpp/`)

A more feature-rich C++ implementation with comprehensive NFA and DFA abstractions. For detailed information about building and running the C++ version, see [cpp/README.md](cpp/README.md).

**Building:**

```shell
cd cpp
make
```

**Running:**

```shell
./lexer <input-file>.ai
```

## Key Features

- **Lexical Analysis**: Table-driven lexer for tokenizing TinyAI source code
- **Finite Automata**: NFA to DFA conversion for pattern matching
- **Regular Expression Support**: Parse and compile regex patterns
- **Multi-implementation**: Both C and C++ versions available

## Getting Started

1. Choose your preferred implementation (C or C++)
2. Navigate to the respective directory
3. Build using Make or CMake (C++ only)
4. Run the lexer on your TinyAI source files
