#include "ast_node.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

StmtListNode *stmt_list_create()
{
        StmtListNode *list = malloc(sizeof(StmtListNode));
        if (!list) {
                fprintf(
                    stderr,
                    "cannot allocate memory for list in stmt_list_create\n");
                return NULL;
        }

        list->cap = 8;
        list->size = 0;
        list->stmts = malloc(list->cap * sizeof(ASTNode));
        if (!list->stmts) {
                fprintf(stderr,
                        "cannot allocate memory for list->stmts in "
                        "stmt_list_create\n");
                free(list);
                return NULL;
        }

        return list;
}

void stmt_list_free(StmtListNode *list)
{
        if (!list)
                return;

        for (size_t i = 0; i < list->size; i++) {
                ast_node_free(list->stmts[i]);
        }
        free(list->stmts);
        free(list);
}

int stmt_list_add(StmtListNode *list, ASTNode *stmt)
{
        if (!list) {
                fprintf(stderr, "null list passed to stmt_list_add\n");
                return -1;
        }

        if (!stmt) {
                fprintf(stdout, "null stmt ignored\n");
                return 0;
        }

        if (list->size >= list->cap) {
                size_t new_cap = list->cap * 2;
                ASTNode **new_stmts =
                    realloc(list->stmts, new_cap * sizeof(ASTNode *));
                if (!new_stmts) {
                        fprintf(stderr,
                                "cannot reallocate memory for list->stmts in "
                                "stmt_list_add\n");
                        return -1;
                }
                list->stmts = new_stmts;
                list->cap = new_cap;
        }

        list->stmts[list->size++] = stmt;
        return list->size - 1;
}

ASTNode *stmt_list_get(StmtListNode *list, size_t idx)
{
        if (!list || idx >= list->size)
                return NULL;

        return list->stmts[idx];
}

/* helper functions */
static char *strdup_safe(const char *str)
{
        if (!str)
                return NULL;
        char *dup = malloc(strlen(str) + 1);
        if (!dup) {
                fprintf(stderr, "malloc failed in strdup_safe\n");
                return NULL;
        }
        strcpy(dup, str);
        return dup;
}

static ASTNode *node_base_create(NodeType type)
{
        ASTNode *node = malloc(sizeof(ASTNode));
        if (!node) {
                fprintf(stderr, "malloc failed in node_base_create\n");
                return NULL;
        }
        memset(node, 0, sizeof(ASTNode));
        node->type = type;
        node->line = 0;
        node->col = 0;
        return node;
}

/* ASTNode constructors */

ASTNode *node_program_create(StmtListNode *stmt_list)
{
        ASTNode *node = node_base_create(NODE_PROGRAM);
        if (!node) {
                return NULL;
        }

        node->data.stmt_list = stmt_list;
        return node;
}

ASTNode *node_decl_create(DataType type, char *ident, ASTNode *init)
{
        if (!ident) {
                fprintf(stderr, "null ident in node_decl_create\n");
                return NULL;
        }

        ASTNode *node = node_base_create(NODE_DECL);
        if (!node) {
                return NULL;
        }

        DeclNode *decl = malloc(sizeof(DeclNode));
        if (!decl) {
                fprintf(stderr, "malloc failed for decl_node\n");
                free(node);
                return NULL;
        }

        decl->type = type;
        decl->ident = strdup_safe(ident);
        decl->init_expr = init;
        if (!decl->ident) {
                free(decl);
                free(node);
                return NULL;
        }

        node->data.decl = decl;
        return node;
}

/**
 * creates a regular assignment node
 */
ASTNode *node_assign_create(char *ident, ASTNode *expr)
{
        if (!ident || !expr) {
                fprintf(stderr, "null parameter in node_assign_create\n");
                return NULL;
        }

        ASTNode *node = node_base_create(NODE_ASSIGN);
        if (!node) {
                return NULL;
        }

        AssignNode *assign = malloc(sizeof(AssignNode));
        if (!assign) {
                fprintf(stderr, "malloc failed for assignnode\n");
                free(node);
                return NULL;
        }

        assign->ident = strdup_safe(ident);
        assign->expr = expr;
        assign->is_input = false;
        assign->input_prompt = NULL;
        if (!assign->ident) {
                free(assign);
                free(node);
                return NULL;
        }

        node->data.assign = assign;
        return node;
}

/**
 * creates an input assignment node
 */
ASTNode *node_input_assign_create(char *ident, char *prompt)
{
        if (!ident || !prompt) {
                fprintf(stderr, "null parameter in node_input_assign_create\n");
                return NULL;
        }

        ASTNode *node = node_base_create(NODE_INPUT);
        if (!node) {
                return NULL;
        }

        AssignNode *assign = malloc(sizeof(AssignNode));
        if (!assign) {
                fprintf(stderr, "malloc failed for input assignnode\n");
                free(node);
                return NULL;
        }

        assign->ident = strdup_safe(ident);
        assign->expr = NULL;
        assign->is_input = true;
        assign->input_prompt = strdup_safe(prompt);
        if (!assign->ident || !assign->input_prompt) {
                free(assign->ident);
                free(assign->input_prompt);
                free(assign);
                free(node);
                return NULL;
        }

        node->data.assign = assign;
        return node;
}

ASTNode *node_if_create(ASTNode *cond,
                        ASTNode *if_stmt,
                        ElifNode *elif_list,
                        ASTNode *else_stmt)
{
        if (!cond || !if_stmt) {
                fprintf(stderr, "null cond or if_stmt in node_if_create\n");
                return NULL;
        }

        ASTNode *node = node_base_create(NODE_IF);
        if (!node) {
                return NULL;
        }

        IfNode *if_node = malloc(sizeof(IfNode));
        if (!if_node) {
                fprintf(stderr, "malloc failed for ifnode\n");
                free(node);
                return NULL;
        }

        if_node->cond = cond;
        if_node->if_stmt = if_stmt;
        if_node->elif_list = elif_list;
        if_node->else_stmt = else_stmt;

        node->data.if_stmt = if_node;
        return node;
}

ASTNode *node_while_create(ASTNode *cond, ASTNode *body)
{
        if (!cond || !body) {
                fprintf(stderr, "null cond or body in node_while_create\n");
                return NULL;
        }

        ASTNode *node = node_base_create(NODE_WHILE);
        if (!node) {
                return NULL;
        }

        WhileNode *while_node = malloc(sizeof(WhileNode));
        if (!while_node) {
                fprintf(stderr, "malloc failed for whilenode\n");
                free(node);
                return NULL;
        }

        while_node->cond = cond;
        while_node->body = body;

        node->data.while_stmt = while_node;
        return node;
}

ASTNode *
node_for_create(ASTNode *init, ASTNode *cond, ASTNode *iter, ASTNode *body)
{
        if (!body) {
                fprintf(stderr, "null body in node_for_create\n");
                return NULL;
        }

        ASTNode *node = node_base_create(NODE_FOR);
        if (!node) {
                return NULL;
        }

        ForNode *for_node = malloc(sizeof(ForNode));
        if (!for_node) {
                fprintf(stderr, "malloc failed for fornode\n");
                free(node);
                return NULL;
        }

        for_node->init = init;
        for_node->cond = cond;
        for_node->iter = iter;
        for_node->body = body;

        node->data.for_stmt = for_node;
        return node;
}

ASTNode *node_print_create(ASTNode *expr)
{
        if (!expr) {
                fprintf(stderr, "null expr in node_print_create\n");
                return NULL;
        }

        ASTNode *node = node_base_create(NODE_PRINT);
        if (!node) {
                return NULL;
        }

        PrintNode *print = malloc(sizeof(PrintNode));
        if (!print) {
                fprintf(stderr, "malloc failed for printnode\n");
                free(node);
                return NULL;
        }

        print->expr = expr;

        node->data.print_stmt = print;
        return node;
}

ASTNode *node_binary_op_create(Operator op, ASTNode *left, ASTNode *right)
{
        if (!left || !right) {
                fprintf(stderr, "null operator in node_binary_op_create\n");
                return NULL;
        }

        ASTNode *node = node_base_create(NODE_BINARY_OP);
        if (!node) {
                return NULL;
        }

        BinaryOpNode *binop = malloc(sizeof(BinaryOpNode));
        if (!binop) {
                fprintf(stderr, "malloc failed for binaryopnode\n");
                free(node);
                return NULL;
        }

        binop->op = op;
        binop->left = left;
        binop->right = right;

        node->data.bin_expr = binop;
        return node;
}

ASTNode *node_unary_op_create(Operator op, ASTNode *operand)
{
        if (!operand) {
                fprintf(stderr, "null operand in node_unary_op_create\n");
                return NULL;
        }

        ASTNode *node = node_base_create(NODE_UNARY_OP);
        if (!node) {
                return NULL;
        }

        UnaryOpNode *unop = malloc(sizeof(UnaryOpNode));
        if (!unop) {
                fprintf(stderr, "malloc failed for unaryopnode\n");
                free(node);
                return NULL;
        }

        unop->op = op;
        unop->operand = operand;

        node->data.unary_expr = unop;
        return node;
}

ASTNode *node_literal_create(DataType type, LiteralValue val)
{
        ASTNode *node = node_base_create(NODE_LITERAL);
        if (!node) {
                return NULL;
        }

        LiteralNode *lit = malloc(sizeof(LiteralNode));
        if (!lit) {
                fprintf(stderr, "malloc failed for literalnode\n");
                free(node);
                return NULL;
        }

        lit->type = type;
        lit->value = val;

        // if string, ensure copy
        if (type == TYPE_STRING && val.str_val) {
                lit->value.str_val = strdup_safe(val.str_val);
                if (!lit->value.str_val) {
                        free(lit);
                        free(node);
                        return NULL;
                }
        }

        node->data.lit = lit;
        return node;
}

ASTNode *node_ident_create(char *ident)
{
        if (!ident) {
                fprintf(stderr, "null ident in node_ident_create\n");
                return NULL;
        }

        ASTNode *node = node_base_create(NODE_IDENT);
        if (!node) {
                return NULL;
        }

        IdentNode *ident_node = malloc(sizeof(IdentNode));
        if (!ident_node) {
                fprintf(stderr, "malloc failed for identnode\n");
                free(node);
                return NULL;
        }

        ident_node->name = strdup_safe(ident);
        if (!ident_node->name) {
                free(ident_node);
                free(node);
                return NULL;
        }

        node->data.ident = ident_node;
        return node;
}

ASTNode *node_func_call_create(char *func_name, ArgNode *args)
{
        if (!func_name) {
                fprintf(stderr, "null func_name in node_func_call_create\n");
                return NULL;
        }

        ASTNode *node = node_base_create(NODE_FUNC_CALL);
        if (!node) {
                return NULL;
        }

        FuncCallNode *func_call = malloc(sizeof(FuncCallNode));
        if (!func_call) {
                fprintf(stderr, "malloc failed for funccallnode\n");
                free(node);
                return NULL;
        }

        func_call->func_name = strdup_safe(func_name);
        func_call->arg_list = args;
        if (!func_call->func_name) {
                free(func_call);
                free(node);
                return NULL;
        }

        node->data.func_call = func_call;
        return node;
}

ASTNode *node_stmt_block_create(StmtListNode *stmts)
{
        ASTNode *node = node_base_create(NODE_STMT_BLOCK);
        if (!node) {
                return NULL;
        }

        node->data.stmt_list = stmts;
        return node;
}

/* linked list helpers for elif and arg lists */

ElifNode *elif_node_create(ASTNode *cond, ASTNode *stmt, ElifNode *next)
{
        if (!cond || !stmt) {
                fprintf(stderr, "null cond or stmt in elif_node_create\n");
                return NULL;
        }

        ElifNode *elif = malloc(sizeof(ElifNode));
        if (!elif) {
                fprintf(stderr, "malloc failed for elif node\n");
                return NULL;
        }

        elif->cond = cond;
        elif->stmt = stmt;
        elif->next = next;

        return elif;
}

ArgNode *arg_node_create(ASTNode *expr, ArgNode *next)
{
        if (!expr) {
                fprintf(stderr, "null expr in arg_node_create\n");
                return NULL;
        }

        ArgNode *arg = malloc(sizeof(ArgNode));
        if (!arg) {
                fprintf(stderr, "malloc failed for argnode\n");
                return NULL;
        }

        arg->expr = expr;
        arg->next = next;

        return arg;
}

/* memory management */

static void elif_list_free(ElifNode *elif)
{
        while (elif) {
                ElifNode *next = elif->next;
                ast_node_free(elif->cond);
                ast_node_free(elif->stmt);
                free(elif);
                elif = next;
        }
}

static void arg_list_free(ArgNode *arg)
{
        while (arg) {
                ArgNode *next = arg->next;
                ast_node_free(arg->expr);
                free(arg);
                arg = next;
        }
}

void ast_node_free(ASTNode *node)
{
        if (!node) {
                return;
        }

        switch (node->type) {
        case NODE_PROGRAM:
        case NODE_STMT_BLOCK:
                stmt_list_free(node->data.stmt_list);
                break;

        case NODE_DECL:
                free(node->data.decl->ident);
                ast_node_free(node->data.decl->init_expr);
                free(node->data.decl);
                break;

        case NODE_ASSIGN:
        case NODE_INPUT:
                free(node->data.assign->ident);
                ast_node_free(node->data.assign->expr);
                free(node->data.assign->input_prompt);
                free(node->data.assign);
                break;

        case NODE_IF:
                ast_node_free(node->data.if_stmt->cond);
                ast_node_free(node->data.if_stmt->if_stmt);
                elif_list_free(node->data.if_stmt->elif_list);
                ast_node_free(node->data.if_stmt->else_stmt);
                free(node->data.if_stmt);
                break;

        case NODE_WHILE:
                ast_node_free(node->data.while_stmt->cond);
                ast_node_free(node->data.while_stmt->body);
                free(node->data.while_stmt);
                break;

        case NODE_FOR:
                ast_node_free(node->data.for_stmt->init);
                ast_node_free(node->data.for_stmt->cond);
                ast_node_free(node->data.for_stmt->iter);
                ast_node_free(node->data.for_stmt->body);
                free(node->data.for_stmt);
                break;

        case NODE_PRINT:
                ast_node_free(node->data.print_stmt->expr);
                free(node->data.for_stmt);
                break;

        case NODE_BINARY_OP:
                ast_node_free(node->data.bin_expr->left);
                ast_node_free(node->data.bin_expr->right);
                free(node->data.bin_expr);
                break;

        case NODE_UNARY_OP:
                ast_node_free(node->data.unary_expr->operand);
                free(node->data.unary_expr);
                break;

        case NODE_LITERAL:
                if (node->data.lit->type == TYPE_STRING) {
                        free(node->data.lit->value.str_val);
                }
                free(node->data.lit);
                break;

        case NODE_IDENT:
                free(node->data.ident->name);
                free(node->data.ident);
                break;

        case NODE_FUNC_CALL:
                free(node->data.func_call->func_name);
                arg_list_free(node->data.func_call->arg_list);
                free(node->data.func_call);
                break;

        default:
                fprintf(stderr, "unknown node type in ast_node_free\n");
                break;
        }

        free(node);
}