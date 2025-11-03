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
  - **engine.hpp**: Header file for combining regex patterns.
  - **engine.cpp**: Implementation of the regex combination logic.

- **include/**: Contains public interface headers for the lexer.
  - **table_driven_lexer/**: Namespace for the lexer components.
    - **regex_parser.hpp**: Public interface for regex parsing.
    - **nfa.hpp**: Public interface for NFA functionality.
    - **dfa.hpp**: Public interface for DFA functionality.

- **examples/**: Contains example implementations.
  - **combine_regexes.cpp**: Demonstrates how to combine multiple regex patterns.

- **tests/**: Contains unit tests for the project.
  - **test_regex.cpp**: Tests for regex parsing and NFA/DFA functionalities.
  - **CMakeLists.txt**: Configuration for building tests.

- **CMakeLists.txt**: Main configuration file for building the project using CMake.

- **Makefile**: Defines build commands for the project.

## Building the Project

To build the project, you can use either CMake or Make. 

### Using CMake

1. Create a build directory:
   ```
   mkdir build
   cd build
   ```

2. Run CMake to configure the project:
   ```
   cmake ..
   ```

3. Build the project:
   ```
   make
   ```

### Using Make

Simply run:
```
make
```

## Running the Application

After building the project, you can run the application by executing the compiled binary. The application will combine the specified regex patterns into a single e-NFA, convert it to a DFA, and test various input strings for matches.

## Testing

To run the tests, navigate to the `tests` directory and use CMake to build and run the tests:
```
cd tests
cmake .
make
./test_regex
```

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for any enhancements or bug fixes.

## License

This project is licensed under the MIT License. See the LICENSE file for details.