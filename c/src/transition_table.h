#ifndef TRANSITION_TABLE_H
#define TRANSITION_TABLE_H

#include <limits.h>

#define STATE_COUNT 425
#define SYMBOL_COUNT 128

extern const char ALPHABET[SYMBOL_COUNT];
extern const int SYMBOL_TO_ID[256];
extern const int TRANSITION_TABLE[STATE_COUNT][SYMBOL_COUNT];

#define START_STATE_ID 0

extern const int ACCEPT_STATE_IDS[STATE_COUNT];

typedef enum {
        AND = 35,
        ARRAY_TOK = 61,
        ASSIGN = 14,
        ASTERISK = 5,
        AS_TOK = 28,
        BOOL_LITERAL = 58,
        BOOL_TOK = 47,
        BREAK_TOK = 62,
        CHAR_LITERAL = 33,
        CHAR_TOK = 48,
        COLON = 11,
        COMMA = 7,
        COMMENT = 1,
        CONCAT_TOK = 71,
        CONTINUE_TOK = 79,
        DO = 29,
        DOT_TOK = 36,
        DOUBLE_ASTERISK = 23,
        DOUBLE_SLASH = 24,
        ELIF_TOK = 49,
        ELSE_TOK = 50,
        END = 37,
        EQUAL = 26,
        FILTER_TOK = 72,
        FLATTEN_TOK = 78,
        FLOAT_LITERAL = 34,
        FLOAT_TOK = 63,
        FOR_TOK = 38,
        FROM_TOK = 51,
        FUNC_TOK = 52,
        GREATER_EQUAL = 27,
        GREATER_THAN = 15,
        IDENTIFIER = 16,
        IF_TOK = 30,
        IMPORT_TOK = 73,
        INPUT_TOK = 64,
        INT_LITERAL = 10,
        INT_TOK = 39,
        IN_TOK = 31,
        LEFT_CURLY_BRACE = 19,
        LEFT_PARENTHESIS = 3,
        LEFT_SQUARE_BRACKET = 17,
        LESS_EQUAL = 25,
        LESS_THAN = 13,
        MATRIX_TOK = 74,
        MAX_TOK = 40,
        MEAN_TOK = 53,
        MINUS = 8,
        MIN_TOK = 41,
        MODULO = 2,
        MULTILINE_COMMENT = 70,
        NORMALIZE_TOK = 82,
        NOT = 42,
        NOT_EQUAL = 21,
        NUMPY_TOK = 65,
        ONES_TOK = 54,
        OR = 32,
        PLUS = 6,
        PRINT_TOK = 66,
        RAND_TOK = 55,
        READCSV_TOK = 80,
        RETURN_TOK = 75,
        RIGHT_CURLY_BRACE = 20,
        RIGHT_PARENTHESIS = 4,
        RIGHT_SQUARE_BRACKET = 18,
        SEMI_COLON = 12,
        SLASH = 9,
        SLICE_TOK = 67,
        SORT_TOK = 56,
        STD_TOK = 43,
        STRING_LITERAL = 22,
        STRING_TOK = 76,
        SUM_TOK = 44,
        TENSOR_TOK = 77,
        THEN = 57,
        TOARRAY_TOK = 81,
        TOTENSOR_TOK = 83,
        USE_TOK = 45,
        VAR_TOK = 46,
        VOID_TOK = 59,
        WHILE_TOK = 68,
        WHITESPACE = 0,
        WITH_TOK = 60,
        ZEROS_TOK = 69,
        UNKNOWN = INT_MAX,
        TOKEN_TYPE_COUNT = 84
} TokenType;

extern const int STATE_TOKEN_TYPE[STATE_COUNT];

#endif // TRANSITION_TABLE_H
