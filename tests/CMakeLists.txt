set(CTest)
enable_testing()

add_executable(test_regex test_regex.cpp)

target_link_libraries(test_regex PRIVATE table_driven_lexer)

add_test(NAME RegexTests COMMAND test_regex)