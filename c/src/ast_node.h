#ifndef AST_NODE_H
#define AST_NODE_H

typedef struct ASTNode ASTNode;

typedef enum DataType {
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_BOOL,
        TYPE_CHAR,
        TYPE_STRING,
} DataType;

typedef enum Operator {
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_MOD,
        OP_INTDIV,
        OP_POW,
        OP_EQ,
        OP_NEQ,
        OP_LT,
        OP_LTEQ,
        OP_GT,
        OP_GTEQ,
        OP_AND,
        OP_OR,
        OP_NOT,
        OP_NEG,
} Operator;

#endif