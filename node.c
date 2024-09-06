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

struct ast_node* create_function_definition_node(struct pos pos, const char* return_type, const char* name, struct ast_node** parameters, int param_count, struct ast_node* body) {
    struct ast_node* node = create_ast_node(AST_FUNCTION_DEFINITION, pos);
    node->function_def.return_type = strdup(return_type);
    node->function_def.name = strdup(name);
    node->function_def.parameters = parameters;
    node->function_def.param_count = param_count;
    node->function_def.body = body;
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

struct ast_node* create_print_node(struct pos pos, struct ast_node* expression) {
    struct ast_node* node = create_ast_node(AST_PRINT, pos);
    node->print.expression = expression;
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

void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}



void print_ast_node(struct ast_node* node, int indent) {
    if (node == NULL) return;

    print_indent(indent);

    switch (node->type) {
        case AST_ROOT:
            printf("ROOT\n");
            for (int i = 0; i < node->root.stmt_count; i++) {
                print_ast_node(node->root.statements[i], indent + 1);
            }
            break;
        case AST_IDENTIFIER:
        case AST_NUMBER:
        case AST_STRING:
        case AST_CHAR:
        case AST_LITERAL:
        case AST_VARIABLE:
            printf("%s: %s\n", 
                node->type == AST_IDENTIFIER ? "IDENTIFIER" :
                node->type == AST_NUMBER ? "NUMBER" :
                node->type == AST_STRING ? "STRING" :
                // node -> type == AST_VARIABLE? "VARIABLE":
                node->type == AST_CHAR ? "CHAR" : "LITERAL",
                node->id_literal.value);
            break;
        case AST_BINARY_OP:
            printf("BINARY_OP: %s\n", node->binary_op.operator);
            print_ast_node(node->binary_op.left, indent + 1);
            print_ast_node(node->binary_op.right, indent + 1);
            break;
        case AST_UNARY_OP:
            printf("UNARY_OP: %s (%s)\n", node->unary_op.operator,
                   node->unary_op.is_postfix ? "postfix" : "prefix");
            print_ast_node(node->unary_op.operand, indent + 1);
            break;
        case AST_FUNCTION_CALL:
            printf("FUNCTION_CALL: %s\n", node->function_call.name);
            for (int i = 0; i < node->function_call.arg_count; i++) {
                print_ast_node(node->function_call.arguments[i], indent + 1);
            }
            break;
        case AST_DECLARATION:
            printf("DECLARATION: %s %s\n", node->declaration.type, node->declaration.name);
            if (node->declaration.initial_value) {
                print_ast_node(node->declaration.initial_value, indent + 1);
            }
            break;
        case AST_IF_STMT:
            printf("IF_STATEMENT\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast_node(node->if_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("Then:\n");
            print_ast_node(node->if_stmt.true_body, indent + 2);
            if (node->if_stmt.false_body) {
                print_indent(indent + 1);
                printf("Else:\n");
                print_ast_node(node->if_stmt.false_body, indent + 2);
            }
            break;
        case AST_WHILE_LOOP:
            printf("WHILE_LOOP\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast_node(node->while_loop.condition, indent + 2);
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast_node(node->while_loop.body, indent + 2);
            break;
        case AST_FOR:
            printf("FOR_LOOP\n");
            print_indent(indent + 1);
            printf("Init:\n");
            print_ast_node(node->for_loop.init, indent + 2);
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast_node(node->for_loop.condition, indent + 2);
            print_indent(indent + 1);
            printf("Update:\n");
            print_ast_node(node->for_loop.update, indent + 2);
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast_node(node->for_loop.body, indent + 2);
            break;
        case AST_RETURN:
            printf("RETURN\n");
            if (node->return_stmt.value) {
                print_ast_node(node->return_stmt.value, indent + 1);
            }
            break;
        case AST_BLOCK:
            printf("BLOCK\n");
            for (int i = 0; i < node->block.stmt_count; i++) {
                print_ast_node(node->block.statements[i], indent + 1);
            }
            break;
        case AST_FUNCTION_DEFINITION:
            printf("FUNCTION_DEFINITION: %s %s\n", node->function_def.return_type, node->function_def.name);
            print_indent(indent + 1);
            printf("Parameters:\n");
            for (int i = 0; i < node->function_def.param_count; i++) {
                print_ast_node(node->function_def.parameters[i], indent + 2);
            }
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast_node(node->function_def.body, indent + 2);
            break;
        case AST_PREPROCESSOR:
            printf("PREPROCESSOR: %s\n", node->id_literal.value);
            break;
        case AST_PRINT:
            printf("PRINT:\n  \tExpression:\n");
            // print_indent(indent + 1);
            // printf("");
            print_ast_node(node->print.expression, indent + 2);
            break;
        default:
            printf("UNKNOWN NODE TYPE: %d\n", node->type);
    }
}

void print_ast(struct ast_node* root) {
    if (root == NULL) {
        printf("Empty AST\n");
        return;
    }
    print_ast_node(root, 0);
}
