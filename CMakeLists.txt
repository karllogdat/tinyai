cmake_minimum_required(VERSION 3.10)

project(TableDrivenLexer)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

file(GLOB SOURCES
    src/*.cpp
)

add_executable(table_driven_lexer ${SOURCES})

enable_testing()
add_subdirectory(tests)