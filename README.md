# Table Driven Lexer

This project implements a table-driven lexer that combines multiple regular expressions (regex) into a single epsilon-NFA (e-NFA), converts it to a deterministic finite automaton (DFA), and tests the matching functionality against various input strings.

## Project Structure

- **src/**: Contains the source code files.
  - **main.cpp**: Entry point of the application.
  - **regex_parser.hpp**: Header file for the regex lexer and parser.
  - **regex_parser.cpp**: Implementation of the regex lexer and parser.
  - **nfa.hpp**: Header file for the NFA representation.
  - **nfa.cpp**: Implementation of the NFA functionality.
  - **dfa.hpp**: Header file for the DFA representation.
  - **dfa.cpp**: Implementation of the DFA functionality.
  - **lexer.hpp**: Header file for lexical analyzer.
  - **lexer.cpp**: Implementation file for lexical analyzer.

- **CMakeLists.txt**: Main configuration file for building the project using CMake.

- **Makefile**: Defines build commands for the project.

## Building the Project

To build the project, you can use either CMake or Make.

### Using CMake

1. Create a build directory:

   ```shell
   mkdir build
   cd build
   ```

2. Run CMake to configure the project:

   ```shell
   cmake ..
   ```

3. Build the project:

   ```shell
   make
   ```

### Using Make

Simply run:

```shell
make
```

## Running the Application

After building the project, you can run the application by executing the compiled binary. The application will combine the specified regex patterns into a single e-NFA, convert it to a DFA, and test various input strings for matches.

```shell
.\lexer.exe .\test.ai
```

Alternatively, follow the following format if lexing other `.ai` files.

```shell
.\lexer <input-file>
```

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for any enhancements or bug fixes.

## License

This project is licensed under the MIT License. See the LICENSE file for details.
