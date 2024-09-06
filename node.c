#include "compiler.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "./helpers/vector.h"
#include <string.h>


// AST node creation functions
struct ast_node* create_ast_node(enum ast_node_type type, struct pos pos) {
    struct ast_node* node = calloc(1, sizeof(struct ast_node));
    node->type = type;
    node->pos = pos;
    return node;
}
struct ast_node* create_id_literal_node(enum ast_node_type type, struct pos pos, const char* value) {
    struct ast_node* node = malloc(sizeof(struct ast_node));
    node->type = type;
    node->pos = pos;
    node->id_literal.value = strdup(value);
    return node;
}

struct ast_node* create_binary_op_node(struct pos pos, struct ast_node* left, struct ast_node* right, const char* operator) {
    struct ast_node* node = malloc(sizeof(struct ast_node));
    node->type = AST_BINARY_OP;
    node->pos = pos;
    node->binary_op.left = left;
    node->binary_op.right = right;
    node->binary_op.operator = strdup(operator);
    return node;
}

struct ast_node* create_function_call_node(struct pos pos, const char* name, struct ast_node** arguments, int arg_count) {
    struct ast_node* node = malloc(sizeof(struct ast_node));
    node->type = AST_FUNCTION_CALL;
    node->pos = pos;
    node->function_call.name = strdup(name);
    node->function_call.arguments = arguments;
    node->function_call.arg_count = arg_count;
    return node;
}

struct ast_node* create_declaration_node(struct pos pos, const char* type, const char* name, struct ast_node* initial_value) {
    struct ast_node* node = malloc(sizeof(struct ast_node));
    node->type = AST_DECLARATION;
    node->pos = pos;
    node->declaration.type = strdup(type);
    node->declaration.name = strdup(name);
    node->declaration.initial_value = initial_value;
    return node;
}

struct ast_node* create_block_node(struct pos pos) {
    struct ast_node* node = malloc(sizeof(struct ast_node));
    node->type = AST_BLOCK;
    node->pos = pos;
    node->block.statements = NULL;
    node->block.stmt_count = 0;
    return node;
}

// Function to create an if statement node
struct ast_node* create_if_stmt_node(struct pos pos, struct ast_node* condition, struct ast_node* true_body, struct ast_node* false_body) {
    struct ast_node* node = malloc(sizeof(struct ast_node));
    node->type = AST_IF_STMT;
    node->pos = pos;
    node->if_stmt.condition = condition;
    node->if_stmt.true_body = true_body;
    node->if_stmt.false_body = false_body;
    return node;
}

// Update the create_while_loop_node function
struct ast_node* create_while_loop_node(struct pos pos, struct ast_node* condition, struct ast_node* body) {
    struct ast_node* node = malloc(sizeof(struct ast_node));
    node->type = AST_WHILE_LOOP;
    node->pos = pos;
    node->while_loop.condition = condition;
    node->while_loop.body = body;
    return node;
}



void add_child(struct ast_node* parent, struct ast_node* child) {
    if (!parent || !child) return;

    switch (parent->type) {
        case AST_PROGRAM:
        case AST_BLOCK:
            parent->block.statements = realloc(parent->block.statements, 
                                               (parent->block.stmt_count + 1) * sizeof(struct ast_node*));
            parent->block.statements[parent->block.stmt_count++] = child;
            break;
        case AST_IF_STMT:
            if (!parent->if_stmt.true_body) {
                parent->if_stmt.true_body = child;
            } else if (!parent->if_stmt.false_body) {
                parent->if_stmt.false_body = child;
            }
            break;
        case AST_WHILE_LOOP:
            if (!parent->while_loop.body) {
                parent->while_loop.body = child;
            }
            break;
        // Add more cases as needed
        default:
            compiler_error(NULL, "Attempt to add child to node type that doesn't support children");
            break;
    }
}

struct ast_node* create_ast_node_with_value(enum ast_node_type type, const char* value, struct pos pos) {
    struct ast_node* node = create_ast_node(type, pos);
    if (node) {
        switch (type) {
            case AST_IDENTIFIER:
            case AST_NUMBER:
            case AST_STRING:
            case AST_CHAR:
                node->id_literal.value = strdup(value);
                break;
            default:
                compiler_error(NULL, "Attempt to create node with value for type that doesn't support it");
                free(node);
                return NULL;
        }
    }
    return node;
}
void free_ast_node(struct ast_node* node) {
    if (!node) return;

    switch (node->type) {
        case AST_IDENTIFIER:
        case AST_NUMBER:
        case AST_STRING:
        case AST_CHAR:
            free(node->id_literal.value);
            break;
        case AST_BINARY_OP:
            free_ast_node(node->binary_op.left);
            free_ast_node(node->binary_op.right);
            free(node->binary_op.operator);
            break;
        
        case AST_FUNCTION_CALL:
            free(node->function_call.name);
            for (int i = 0; i < node->function_call.arg_count; i++) {
                free_ast_node(node->function_call.arguments[i]);
            }
            free(node->function_call.arguments);
            break;
        // Add cases for other node types...
    }

    free(node);
}