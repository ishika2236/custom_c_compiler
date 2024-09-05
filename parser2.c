#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include "./helpers/vector.h"
#include <string.h>
// Function to parse variable declarations
static void parse_variable_declaration(struct parse_process* parser, struct ast_node* parent_node) {
    // Expect variable type
    struct token* type_token = get_next_token(parser);
    if (type_token->type != TOKEN_TYPE_IDENTIFIER) {
        fprintf(stderr, "Error: Expected type identifier for variable declaration.\n");
        exit(EXIT_FAILURE);
    }
    char* type_value = type_token->sval;
    
    // Create a new AST node for the variable declaration
    struct ast_node* var_decl_node = create_ast_node(AST_STATEMENT);
    add_child(parent_node, var_decl_node);
    
    // Create AST node for type
    struct ast_node* type_node = create_ast_node_with_value(AST_IDENTIFIER, type_value);
    add_child(var_decl_node, type_node);

    // Expect variable name
    struct token* identifier_token = get_next_token(parser);
    if (identifier_token->type != TOKEN_TYPE_IDENTIFIER) {
        fprintf(stderr, "Error: Expected variable identifier.\n");
        exit(EXIT_FAILURE);
    }
    char* identifier_value = identifier_token->sval;
    
    // Create AST node for variable name
    struct ast_node* identifier_node = create_ast_node_with_value(AST_IDENTIFIER, identifier_value);
    add_child(var_decl_node, identifier_node);
    
    // Optional initialization
    if (peek_next_token(parser)->type == TOKEN_TYPE_SYMBOL && peek_next_token(parser)->sval[0] == '=') {
        get_next_token(parser);  // Consume '='
        
        // Create AST node for assignment
        struct ast_node* assignment_node = create_ast_node(AST_ASSIGNMENT);
        add_child(var_decl_node, assignment_node);
        
        // Parse and add initialization expression
        parse_expression(parser, assignment_node);
    }
    
    // Expect ';'
    struct token* semicolon_token = get_next_token(parser);
    if (semicolon_token->type != TOKEN_TYPE_SYMBOL || semicolon_token->sval[0] != ';') {
        fprintf(stderr, "Error: Expected ';' at the end of variable declaration.\n");
        exit(EXIT_FAILURE);
    }
}

// Function to parse function declarations
static void parse_function_declaration(struct parse_process* parser, struct ast_node* parent_node) {
    // Expect function return type
    struct token* type_token = get_next_token(parser);
    if (type_token->type != TOKEN_TYPE_IDENTIFIER) {
        fprintf(stderr, "Error: Expected return type for function declaration.\n");
        exit(EXIT_FAILURE);
    }
    char* return_type = type_token->sval;
    
    // Create a new AST node for the function declaration
    struct ast_node* func_decl_node = create_ast_node(AST_STATEMENT);
    add_child(parent_node, func_decl_node);
    
    // Create AST node for return type
    struct ast_node* type_node = create_ast_node_with_value(AST_IDENTIFIER, return_type);
    add_child(func_decl_node, type_node);
    
    // Expect function name
    struct token* identifier_token = get_next_token(parser);
    if (identifier_token->type != TOKEN_TYPE_IDENTIFIER) {
        fprintf(stderr, "Error: Expected function name.\n");
        exit(EXIT_FAILURE);
    }
    char* function_name = identifier_token->sval;
    
    // Create AST node for function name
    struct ast_node* identifier_node = create_ast_node_with_value(AST_IDENTIFIER, function_name);
    add_child(func_decl_node, identifier_node);
    
    // Parse function parameters
    if (peek_next_token(parser)->type == TOKEN_TYPE_SYMBOL && peek_next_token(parser)->sval[0] == '(') {
        get_next_token(parser);  // Consume '('
        parse_parameters(parser, func_decl_node);
        if (peek_next_token(parser)->type == TOKEN_TYPE_SYMBOL && peek_next_token(parser)->sval[0] == ')') {
            get_next_token(parser);  // Consume ')'
        } else {
            fprintf(stderr, "Error: Expected ')' after function parameters.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Parse function body
    parse_compound_statement(parser, func_decl_node);
}


static void parse_return_statement(struct parse_process* parser, struct ast_node* parent_node, const char* expected_return_type) {
    // Create an AST node for the return statement
    struct ast_node* return_node = create_ast_node(AST_STATEMENT);
    add_child(parent_node, return_node);
    
    // Create a node for 'return' keyword
    struct ast_node* return_keyword_node = create_ast_node_with_value(AST_IDENTIFIER, "return");
    add_child(return_node, return_keyword_node);
    
    // Parse the return expression
    struct ast_node* return_expr_node = create_ast_node(AST_EXPRESSION);
    add_child(return_node, return_expr_node);
    
    parse_expression(parser, return_expr_node);
    
    // Check type compatibility (you'll need to implement this check)
    struct token* return_value_token = peek_next_token(parser);
    if (!check_type_compatibility(expected_return_type, return_value_token->type)) {
        fprintf(stderr, "Error: Type mismatch in return statement. Expected %s but got %d.\n", expected_return_type, return_value_token->type);
        exit(EXIT_FAILURE);
    }
    
    // Expect ';'
    struct token* semicolon_token = get_next_token(parser);
    if (semicolon_token->type != TOKEN_TYPE_SYMBOL || semicolon_token->sval[0] != ';') {
        fprintf(stderr, "Error: Expected ';' at the end of return statement.\n");
        exit(EXIT_FAILURE);
    }
}

struct ast_node* parse_program(struct parse_process* parser) {
    struct ast_node* root = create_ast_node(AST_PROGRAM); // Root node of the AST

    while (parser->index < vector_total(parser->token_vector)) {
        struct token* token = vector_get(parser->token_vector, parser->index);
        struct ast_node* node = NULL;

        switch (token->type) {
            case TOKEN_TYPE_KEYWORD:
                // printf("%s\n", token->sval);
                if (is_declaration(token)) {
                    // Handle variable/function declarations
                    node = parse_declaration(parser);
                } 
                else if( strcmp(token->sval, "return") == 0)
                {
                    node =  parse_return_statement(parser);
                }
                else if( strcmp(token->sval, "if") ==0 ||
                         strcmp(token->sval, "for") == 0||
                         strcmp(token -> sval, "while") == 0)
                         {
                            node = parse_control_structure(parser);
                         }
                break;

            case TOKEN_TYPE_NEWLINE:
                printf("Newline encountered \n");
                break;

        
            // case TOKEN_TYPE_COMMENT:
            //     // Skip newlines and comments
            //     process->index++;
            //     continue;

            // case TOKEN_TYPE_EOF:
            //     // End of file, stop parsing
            //     process->index++;
            //     break;

            default:
                syntax_error("Unexpected token", token);
                break;
        }

        // Add the newly created node to the root's children array
        if (node != NULL) {
            add_child(root, node);
        }

        parser->index++; // Move to the next token
    }
    return root;
}
