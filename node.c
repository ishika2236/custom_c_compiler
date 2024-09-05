#include "compiler.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "./helpers/vector.h"
#include <string.h>

struct ast_node* create_ast_node(enum ast_node_type type) {
    struct ast_node* node = (struct ast_node*)malloc(sizeof(struct ast_node));
    node->type = type;
    node->value = NULL; 
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    return node;
}
struct ast_node* create_ast_node_with_value(enum ast_node_type type, const char* value) {
    struct ast_node* node = (struct ast_node*)malloc(sizeof(struct ast_node));
    if (node == NULL) {
        perror("Failed to allocate memory for AST node");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->value = strdup(value);
    if (node->value == NULL) {
        perror("Failed to allocate memory for AST node value");
        exit(EXIT_FAILURE);
    }
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    return node;
}

void add_child(struct ast_node* parent, struct ast_node* child) {
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity = parent->child_capacity == 0 ? 2 : parent->child_capacity * 2;
        parent->children = realloc(parent->children, parent->child_capacity * sizeof(struct ast_node*));
    }
    parent->children[parent->child_count++] = child;
}
void free_ast_node(struct ast_node* node)
{
    if(node -> value)
    {
        free(node -> value);
    }
    for(int i = 0; i < node -> child_count; i++)
    {
        free_ast_node(node -> children[i]);
    }
    free(node -> children);
    free(node);
}

void print_ast(struct ast_node* node, int indent) {
    for (int i = 0; i < indent; ++i) printf("  ");
    
    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            break;
        case AST_STATEMENT:
            printf("Statement\n");
            break;
        case AST_EXPRESSION:
            printf("Expression\n");
            break;
        case AST_TERM:
            printf("Term: %s\n", node->value);
            break;
        case AST_BINARY_OP:
            printf("Binary Operation: %s\n", node->value);
            break;
        case AST_ASSIGNMENT:
            printf("Assignment\n");
            break;
        case AST_IDENTIFIER:
            printf("Identifier: %s\n", node->value);
            break;
        case AST_NUMBER:
            printf("Number: %s\n", node->value);
            break;
        default:
            printf("Unknown Node\n");
            break;
    }
    
    for (int i = 0; i < node->child_count; ++i) {
        print_ast(node->children[i], indent + 1);
    }
}
