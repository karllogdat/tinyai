#include "ast_print.h"

#include <stdio.h>

static const int STEP = 2;

static void print_ast(ASTNode *node, int indent);

static void indent(int level)
{
        for (int i = 0; i < level; i++)
                printf(" ");
}

static const char *op_to_str(Operator op)
{
        switch (op) {
        case OP_ADD:
                return "+";
        case OP_SUB:
                return "-";
        case OP_MUL:
                return "*";
        case OP_DIV:
                return "/";
        case OP_MOD:
                return "%";
        case OP_INTDIV:
                return "//";
        case OP_POW:
                return "**";
        case OP_EQ:
                return "==";
        case OP_NEQ:
                return "!=";
        case OP_LT:
                return "<";
        case OP_LTEQ:
                return "<=";
        case OP_GT:
                return ">";
        case OP_GTEQ:
                return ">=";
        case OP_AND:
                return "and";
        case OP_OR:
                return "or";
        case OP_NOT:
                return "!";
        case OP_NEG:
                return "neg";
        default:
                return "??";
        }
}

static const char *datatype_to_str(DataType t)
{
        switch (t) {
        case TYPE_INT:
                return "int";
        case TYPE_FLOAT:
                return "float";
        case TYPE_BOOL:
                return "bool";
        case TYPE_CHAR:
                return "char";
        case TYPE_STRING:
                return "string";
        default:
                return "unknown";
        }
}

static void print_lit(LiteralNode *lit, int lvl)
{
        indent(lvl);
        printf("Literal(");
        switch (lit->type) {
        case TYPE_INT:
                printf("%d", lit->value.int_val);
                break;
        case TYPE_FLOAT:
                printf("%f", lit->value.float_val);
                break;
        case TYPE_BOOL:
                printf("%s", lit->value.bool_val ? "true" : "false");
                break;
        case TYPE_CHAR:
                printf("%c", lit->value.char_val);
                break;
        case TYPE_STRING:
                printf("%s", lit->value.str_val);
                break;
        }
        printf(")\n");
}

static void print_stmt_list(StmtListNode *list, int lvl)
{
        indent(lvl);
        if (!list) {
                printf("(null stmt list)\n");
                return;
        }

        printf("StmtList (%zu stmts)\n", list->size);

        for (size_t i = 0; i < list->size; i++) {
                print_ast(list->stmts[i], lvl + STEP);
        }
}

static void print_elif(ElifNode *elif, int lvl)
{
        while (elif) {
                indent(lvl);
                printf("Elif:\n");

                indent(lvl + STEP);
                printf("Cond:\n");
                print_ast(elif->cond, lvl + STEP + STEP);

                indent(lvl + STEP);
                printf("Stmt:\n");
                print_ast(elif->stmt, lvl + STEP + STEP);

                elif = elif->next;
        }
}

static void print_args(ArgNode *arg, int lvl)
{
        int idx = 0;
        while (arg) {
                indent(lvl);
                printf("Arg %d:\n", idx++);
                print_ast(arg->expr, lvl + STEP);

                arg = arg->next;
        }
}

static void print_ast(ASTNode *node, int lvl)
{
        if (!node) {
                indent(lvl);
                printf("(null)\n");
                return;
        }

        switch (node->type) {
        case NODE_PROGRAM: {
                indent(lvl);
                printf("Program:\n");
                print_stmt_list(node->data.stmt_list, lvl + STEP);
                break;
        }

        case NODE_STMT_LIST: {
                StmtListNode *list = node->data.stmt_list;
                indent(lvl);
                printf("StmtList (%zu stmts)\n", list->size);

                for (size_t i = 0; i < list->size; i++) {
                        print_ast(list->stmts[i], lvl + STEP);
                }
                break;
        }

        case NODE_STMT_BLOCK: {
                indent(lvl);
                printf("StmtBlock\n");
                print_stmt_list(node->data.stmt_list, lvl + STEP);
                break;
        }

        case NODE_DECL: {
                DeclNode *d = node->data.decl;
                indent(lvl);

                printf("Decl (%s %s)\n", datatype_to_str(d->type), d->ident);
                if (d->init_expr) {
                        indent(lvl + STEP);
                        printf("Init:\n");
                        print_ast(d->init_expr, lvl + STEP + STEP);
                }
                break;
        }

        case NODE_ASSIGN: {
                AssignNode *a = node->data.assign;
                indent(lvl);
                if (a->is_input) {
                        printf("AssignInput(%s)\n", a->ident);
                } else {
                        printf("Assign(%s)\n", a->ident);
                }

                if (a->is_input && a->input_prompt) {
                        indent(lvl + STEP);
                        printf("Prompt: \"%s\"", a->input_prompt);
                }

                if (a->expr) {
                        indent(lvl + STEP);
                        printf("Expr:\n");
                        print_ast(a->expr, lvl + STEP + STEP);
                }
                break;
        }

        case NODE_INPUT: {
                AssignNode *a = node->data.assign;
                indent(lvl);
                printf("InputAssign(%s)\n", a->ident);

                if (a->input_prompt) {
                        indent(lvl + STEP);
                        printf("Prompt: \"%s\"\n", a->input_prompt);
                }
                break;
        }

        case NODE_IF: {
                IfNode *ifn = node->data.if_stmt;
                indent(lvl);
                printf("If:\n");

                indent(lvl + STEP);
                printf("Cond:\n");
                print_ast(ifn->cond, lvl + STEP + STEP);

                indent(lvl + STEP);
                printf("IfStmt:\n");
                print_ast(ifn->if_stmt, lvl + STEP + STEP);

                if (ifn->elif_list) {
                        print_elif(ifn->elif_list, lvl + STEP);
                }

                if (ifn->else_stmt) {
                        indent(lvl + STEP);
                        printf("ElseStmt:\n");
                        print_ast(ifn->else_stmt, lvl + STEP + STEP);
                }
                break;
        }

        case NODE_WHILE: {
                WhileNode *wn = node->data.while_stmt;
                indent(lvl);
                printf("While:\n");

                indent(lvl + STEP);
                printf("Cond:\n");
                print_ast(wn->cond, lvl + STEP + STEP);

                indent(lvl + STEP);
                printf("Body:\n");
                print_ast(wn->body, lvl + STEP + STEP);
                break;
        }

        case NODE_FOR: {
                ForNode *fn = node->data.for_stmt;
                indent(lvl);
                printf("For:\n");

                indent(lvl + STEP);
                printf("Init:\n");
                print_ast(fn->init, lvl + STEP + STEP);

                indent(lvl + STEP);
                printf("Cond:\n");
                print_ast(fn->cond, lvl + STEP + STEP);

                indent(lvl + STEP);
                printf("Iter:\n");
                print_ast(fn->iter, lvl + STEP + STEP);

                indent(lvl + STEP);
                printf("Body:\n");
                print_ast(fn->body, lvl + STEP + STEP);
                break;
        }

        case NODE_PRINT: {
                PrintNode *pn = node->data.print_stmt;
                indent(lvl);
                printf("Print:\n");
                print_ast(pn->expr, lvl + STEP);
                break;
        }

        case NODE_BINARY_OP: {
                BinaryOpNode *bn = node->data.bin_expr;
                indent(lvl);
                printf("BinaryOp(%s):\n", op_to_str(bn->op));

                indent(lvl + STEP);
                printf("Left:\n");
                print_ast(bn->left, lvl + STEP + STEP);

                indent(lvl + STEP);
                printf("Right:\n");
                print_ast(bn->right, lvl + STEP + STEP);
                break;
        }

        case NODE_UNARY_OP: {
                UnaryOpNode *un = node->data.unary_expr;
                indent(lvl);
                printf("UnaryOp(%s):\n", op_to_str(un->op));

                indent(lvl + STEP);
                printf("Operand:\n");
                print_ast(un->operand, lvl + STEP + STEP);
                break;
        }

        case NODE_LITERAL: {
                LiteralNode *ln = node->data.lit;
                print_lit(ln, lvl);
                break;
        }

        case NODE_IDENT: {
                IdentNode *in = node->data.ident;
                indent(lvl);
                printf("Ident(%s)\n", in->name);
                break;
        }

        case NODE_FUNC_CALL: {
                FuncCallNode *fn = node->data.func_call;
                indent(lvl);
                printf("FuncCall(%s):\n", fn->func_name);

                print_args(fn->arg_list, lvl + STEP);
                break;
        }

        default:
                indent(lvl);
                printf("UnknownNode?(%d)\n", node->type);
                break;
        }
}

void ast_print(ASTNode *node)
{
        print_ast(node, 0);
}