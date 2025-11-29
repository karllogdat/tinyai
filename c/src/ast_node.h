#ifndef AST_NODE_H
#define AST_NODE_H

#include <stdbool.h>
#include <stddef.h>

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

typedef enum NodeType {
        NODE_PROGRAM,
        NODE_STMT_LIST,
        NODE_DECL,
        NODE_ASSIGN,
        NODE_IF,
        NODE_WHILE,
        NODE_FOR,
        NODE_PRINT,
        // NODE_EXPR unused since expr have specialized nodes
        NODE_BINARY_OP,
        NODE_UNARY_OP,
        NODE_LITERAL,
        NODE_IDENT,
        NODE_FUNC_CALL,
        NODE_INPUT,
        NODE_STMT_BLOCK,
} NodeType;

typedef union LiteralValue {
        int int_val;
        float float_val;
        bool bool_val;
        char char_val;
        char *str_val;
} LiteralValue;

typedef struct StmtListNode {
        ASTNode **stmts;
        size_t size;
        size_t cap;
} StmtListNode;

StmtListNode *stmt_list_create();
void stmt_list_free(StmtListNode *list);
int stmt_list_add(StmtListNode *list, ASTNode *stmt);
ASTNode *stmt_list_get(StmtListNode *list, size_t idx);

typedef struct DeclNode {
        DataType type;
        char *ident;
        ASTNode *init_expr; // NULL if pure decl stmt
} DeclNode;

typedef struct AssignNode {
        char *ident;
        ASTNode *expr;
        bool is_input;
        // NULL if not input assign
        char *input_prompt;
} AssignNode;

typedef struct ElifNode {
        ASTNode *cond;
        ASTNode *stmt;
        struct ElifNode *next;
} ElifNode;

typedef struct IfNode {
        ASTNode *cond;
        ASTNode *if_stmt;
        ElifNode *elif_list; // uses linked list unlike stmt list dynamic arr
        ASTNode *else_stmt; // NULL if no else
} IfNode;

typedef struct WhileNode {
        ASTNode *cond;
        ASTNode *body;
} WhileNode;

typedef struct ForNode {
        ASTNode *init; // decl, assign, or null
        ASTNode *cond;
        ASTNode *iter; // expr, assign, or null
        ASTNode *body;
} ForNode;

typedef struct PrintNode {
        ASTNode *expr;
} PrintNode;

// expression related nodes

typedef struct BinaryOpNode {
        Operator op;
        ASTNode *left;
        ASTNode *right;
} BinaryOpNode;

typedef struct UnaryOpNode {
        Operator op;
        ASTNode *operand;
} UnaryOpNode;

typedef struct LiteralNode {
        DataType type;
        LiteralValue value;
} LiteralNode;

typedef struct IdentNode {
        char *name;
} IdentNode;

typedef struct ArgNode {
        ASTNode *expr;
        struct ArgNode *next;
} ArgNode;

typedef struct FuncCallNode {
        char *func_name;
        ArgNode *arg_list; // linked list of args
} FuncCallNode;

struct ASTNode {
        NodeType type;
        union {
                StmtListNode *stmt_list;
                DeclNode *decl;
                AssignNode *assign;
                IfNode *if_stmt;
                WhileNode *while_stmt;
                ForNode *for_stmt;
                PrintNode *print_stmt;
                // exprs
                BinaryOpNode *bin_expr;
                UnaryOpNode *unary_expr;
                LiteralNode *lit;
                IdentNode *ident;
                FuncCallNode *func_call;
        } data;

        size_t line;
        size_t col;
};

ASTNode *node_program_create(StmtListNode *stmt_list);
ASTNode *node_stmt_block_create(StmtListNode *stmts);
ASTNode *node_decl_create(DataType type, char *ident, ASTNode *init);
ASTNode *node_assign_create(char *ident, ASTNode *expr);
// handle input assign creation on separate function
ASTNode *node_input_assign_create(char *ident, char *prompt);
ASTNode *node_if_create(ASTNode *cond,
                        ASTNode *if_stmt,
                        ElifNode *elif_list,
                        ASTNode *else_stmt);
ASTNode *node_while_create(ASTNode *cond, ASTNode *body);
ASTNode *
node_for_create(ASTNode *init, ASTNode *cond, ASTNode *iter, ASTNode *body);
ASTNode *node_print_create(ASTNode *expr);
// expression nodes
ASTNode *node_binary_op_create(Operator op, ASTNode *left, ASTNode *right);
ASTNode *node_unary_op_create(Operator op, ASTNode *operand);
ASTNode *node_literal_create(DataType type, LiteralValue val);
ASTNode *node_ident_create(char *ident);
ASTNode *node_func_call_create(char *func_name, ArgNode *args);

// linked list helpers
ElifNode *elif_node_create(ASTNode *cond, ASTNode *stmt, ElifNode *next);
ArgNode *arg_node_create(ASTNode *expr, ArgNode *next);

void elif_list_free(ElifNode *elif);
void arg_list_free(ArgNode *arg);
void ast_node_free(ASTNode *node);

#endif