#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include "./helpers/vector.h"
#include <string.h>

// Function prototypes
struct ast_node* parse_expression(struct parse_process* parser);
struct ast_node* parse_statement(struct parse_process* parser);
struct ast_node* parse_declaration(struct parse_process* parser);
struct ast_node* parse_function_definition(struct parse_process* parser);
struct ast_node* parse_block(struct parse_process* parser);  // Forward declaration

// Helper functions
static struct token* consume_token(struct parse_process* parser) {
    struct token* token = peek_next_token(parser);
    if (token) {
        parser->index++;
        printf("Consumed token: %s\n", token->sval);
    }
    return token;
}

static bool check_and_consume(struct parse_process* parser, int type, const char* value) {
    struct token* token = peek_next_token(parser);
    if (token && token->type == type && 
        ((type == TOKEN_TYPE_SYMBOL && token->cval == value[0]) ||
         (type != TOKEN_TYPE_SYMBOL && strcmp(token->sval, value) == 0))) {
        consume_token(parser);
        printf("Checked and consumed token: %s\n", token->sval);
        return true;
    }
    return false;
}

struct ast_node* parse_primary(struct parse_process* parser) {
    printf("Parsing primary...\n");
    struct token* token = (struct token*) vector_get(parser->token_vector, parser->index);

    // printf("inside primary block %d\n", token->type);
    if (!token) {
        printf("Error: Expected primary expression, but token is NULL\n");
        return NULL;
    }

    struct ast_node* node = NULL;
    switch (token->type) {
        case TOKEN_TYPE_IDENTIFIER:
            node = create_id_literal_node(AST_IDENTIFIER, token->pos, token->sval);
            // printf("Parsed identifier: %s\n", token->sval);
            break;
        case TOKEN_TYPE_NUMBER:
            char* buffer = (char*)malloc(21 * sizeof(char)); 
            sprintf(buffer, "%llu", token -> llnum); 
            node = create_id_literal_node(AST_NUMBER, token->pos, buffer);
            // printf("Parsed number: %lld\n", token->llnum);
            break;
        case TOKEN_TYPE_STRING:
            node = create_id_literal_node(AST_LITERAL, token->pos, token->sval);
            printf("Parsed string literal: %s\n", token->sval);
            break;
        case TOKEN_TYPE_SYMBOL:
            if (token->cval == '(') {
                // printf("Parsed opening parenthesis\n");
                node = parse_expression(parser);
                if (!check_and_consume(parser, TOKEN_TYPE_SYMBOL, ")")) {
                    compiler_error(parser->compiler, "Expected closing parenthesis");
                    printf("Error: Expected closing parenthesis\n");
                    free_ast_node(node);
                    return NULL;
                }
                printf("Parsed closing parenthesis\n");
            }
            break;
        default:
            compiler_error(parser->compiler, "Unexpected token in primary expression");
            printf("Error: Unexpected token in primary expression\n");
            return NULL;
    }
    return node;
}
struct ast_node* parse_print_statement(struct parse_process* parser) {
    printf("Parsing print statement...\n");
    struct token* print_token = consume_token(parser);
    printf("print token: %s\n", print_token->sval);
    // parser->index++;
    struct token* token = (struct token*) vector_get(parser->token_vector, parser->index);
    // printf("%s\n", token->sval);
    if (token -> type !=TOKEN_TYPE_SYMBOL || token->cval != '(') {
        compiler_error(parser->compiler, "Expected opening parenthesis after 'print'");
        printf("Error: Expected opening parenthesis after 'print'\n");
        return NULL;
    }
    parser->index ++;
    
    struct ast_node* expression = parse_expression(parser);
     token = (struct token*) vector_get(parser->token_vector, parser->index);
     printf("%c\n", token->cval);
    if ( token -> type != TOKEN_TYPE_SYMBOL && token -> cval == ')') {
        compiler_error(parser->compiler, "Expected closing parenthesis");
        printf("Error: Expected closing parenthesis in print statement\n");
        free_ast_node(expression);
        return NULL;
    }
    parser -> index++;
    token = (struct token*) vector_get(parser->token_vector, parser->index);
    if (token -> type != TOKEN_TYPE_SYMBOL && token -> cval == ';') {
        compiler_error(parser->compiler, "Expected semicolon after print statement");
        printf("Error: Expected semicolon after print statement\n");
        free_ast_node(expression);
        return NULL;
    }
    
    return create_print_node(print_token->pos, expression);
}
struct ast_node* parse_expression(struct parse_process* parser) {
    printf("Parsing expression...\n");
    struct ast_node* left = parse_primary(parser);
    if (!left) return NULL;

    while (true) {
        struct token* token = peek_next_token(parser);
        if (!token || token->type != TOKEN_TYPE_OPERATOR) break;

        consume_token(parser);
        // printf("Parsed operator: %s\n", token->sval);
        struct ast_node* right = parse_primary(parser);
        if (!right) {
            compiler_error(parser->compiler, "Expected expression after operator");
            printf("Error: Expected expression after operator\n");
            free_ast_node(left);
            return NULL;
        }

        left = create_binary_op_node(token->pos, left, right, token->sval);
    }

    parser->index++;

    return left;
}

struct ast_node* parse_function_definition(struct parse_process* parser) {
    printf("Parsing function definition...\n");
    
    // Consume return type and function name
    struct token* return_type = consume_token(parser);
    struct token* name = consume_token(parser);

    // Expect opening parenthesis
    
    struct token* token = (struct token*) vector_get(parser->token_vector, parser->index);
    parser->index++;
    if (token->type != TOKEN_TYPE_SYMBOL || token->cval != '(') {
        printf("Error: Expected opening parenthesis in function definition\n");
        return NULL;
    }

    // Parse parameters
    struct ast_node** parameters = NULL;
    int param_count = 0;
    
    token = (struct token*) vector_get(parser->token_vector, parser->index);
    // printf(" token val just before param while loop: %s\n", token ->sval);
    parser->index++;
    
    while (token->type != TOKEN_TYPE_SYMBOL || token->cval != ')') {
        // Create and allocate a new parameter
        struct ast_node* param_node = malloc(sizeof(struct ast_node));
        token = (struct token*) vector_get(parser->token_vector, parser->index);
        if (token->type == TOKEN_TYPE_IDENTIFIER) {
            // Assume token->sval contains the parameter name
            // printf("param name %s\n", token->sval);
            param_node->type = AST_VARIABLE;
            param_node->id_literal.value = strdup(token->sval);
        } else {
            printf("Error: Expected parameter name\n");
            return NULL;
        }
        consume_token(parser);
        // Store the parameter in the parameters array
        parameters = realloc(parameters, (param_count + 1) * sizeof(struct ast_node*));
        parameters[param_count++] = param_node;

        // Check for a comma or closing parenthesis
        token = (struct token*) vector_get(parser->token_vector, parser->index);
        parser->index++;
        // printf("%s token type: %d\n", token->sval, token -> type);
        if (token->type == TOKEN_TYPE_SYMBOL && token->cval == ')') {
            // printf("breaking");
            break;
        } 
        // else (token->type == TOKEN_TYPE_OPERATOR  && token->sval == ",") 
        else{
            // Continue to the next parameter
            token = (struct token*) vector_get(parser->token_vector, parser->index);
            // printf("%s\n", token->sval);
            parser->index++;
        }
        // } else {
        //     printf("Error: Expected ',' or ')' after parameter\n");
        //     return NULL;
        // }
    }

    // Parse function body (block of statements)
    struct ast_node* body = parse_block(parser);

    // Return a function call node with the parsed details
    struct ast_node* func_def_node = create_function_definition_node(return_type->pos, return_type->sval, name->sval, parameters, param_count, body);
    return func_def_node;
}


struct ast_node* parse_declaration(struct parse_process* parser) {
    printf("Parsing declaration...\n");
    // printf("%s inside statement funciton", token->sval);
    struct token* type = consume_token(parser);
    struct token* name = consume_token(parser);
    
    struct ast_node* initial_value = NULL;
    struct token* next_token = (struct token*) vector_get(parser->token_vector, parser->index);
    // printf("next_token in declaration in block %c", next_token->cval);
    if (check_and_consume(parser, TOKEN_TYPE_OPERATOR, "=")) {
        initial_value = parse_expression(parser);
    }
    next_token = (struct token*) vector_get(parser->token_vector, parser->index);
    // printf("value of token after parsing expression %c \n", next_token->cval);
    
    if (next_token-> type != TOKEN_TYPE_SYMBOL && next_token-> cval ==';' ) {
        if (initial_value) {
            free_ast_node(initial_value);
        }
        // printf("Error: Expected semicolon after declaration\n");
        return NULL;
    }
    
    return create_declaration_node(type->pos, type->sval, name->sval, initial_value);
}

struct ast_node* parse_block(struct parse_process* parser) {
    printf("Parsing block...\n");
    struct token* token = (struct token*) vector_get(parser->token_vector, parser->index);
    if (token->type != TOKEN_TYPE_SYMBOL || token->cval != '{') {
        compiler_error(parser->compiler, "Expected opening brace");
        printf("Error: Expected opening brace\n");
        return NULL;
    }
    
    struct ast_node* block = create_ast_node(AST_BLOCK, parser->compiler->pos);
    block->block.statements = NULL;
    block->block.stmt_count = 0;

    while (!check_and_consume(parser, TOKEN_TYPE_SYMBOL, "}")) {
        // printf("inside block\n");
        parser->index++;
        struct ast_node* stmt = parse_statement(parser);
        if (stmt) {
            block->block.statements = realloc(block->block.statements, 
                                              (block->block.stmt_count + 1) * sizeof(struct ast_node*));
            block->block.statements[block->block.stmt_count++] = stmt;
        }
        struct token* token = (struct token*) vector_get(parser->token_vector, parser->index);
        // if(token -> type != TOKEN_TYPE_SYMBOL)printf("Values inside parse_block : %s\n", token -> sval);
        if(token-> type == TOKEN_TYPE_SYMBOL && token-> cval =='}')
        {
            parser->index ++;
            printf("semi colon detected in parser_block\n");
            break;
        }
    }

    return block;
}

struct ast_node* parse_preprocessor_directive(struct parse_process* parser) {
    printf("Parsing preprocessor directive...\n");
    struct token* hash = consume_token(parser);
    struct token* directive = consume_token(parser);
    
    if (strcmp(directive->sval, "include") == 0) {
        struct token* file = consume_token(parser);
        return create_ast_node_with_value(AST_PREPROCESSOR, file->sval, hash->pos);
    }
    
    return NULL;
}

struct ast_node* parse_return_statement(struct parse_process* parser) {
    printf("Parsing return statement...\n");
    struct token* return_token = consume_token(parser);
    
    struct ast_node* value = NULL;
    if (!check_and_consume(parser, TOKEN_TYPE_SYMBOL, ";")) {
        value = parse_expression(parser);
        parser -> index ++;
        struct token* token = (struct token*) vector_get(parser->token_vector, parser->index);
        // printf("type of token in return statement %d\n", token->type);
        
        if (token -> type != TOKEN_TYPE_SYMBOL && token -> cval ==';') {
            compiler_error(parser->compiler, "Expected semicolon after return statement");
            printf("Error: Expected semicolon after return statement\n");
            free_ast_node(value);
            return NULL;
        }
    }
    
    struct ast_node* node = create_ast_node(AST_RETURN, return_token->pos);
    node->return_stmt.value = value;
    // printf("returning return node\n");
    return node;
}

struct ast_node* parse_while_statement(struct parse_process* parser) {
    printf("Parsing while statement...\n");
    struct token* while_token = consume_token(parser);
    
    if (!check_and_consume(parser, TOKEN_TYPE_SYMBOL, "(")) {
        compiler_error(parser->compiler, "Expected opening parenthesis");
        printf("Error: Expected opening parenthesis in while statement\n");
        return NULL;
    }
    
    struct ast_node* condition = parse_expression(parser);
    
    if (!check_and_consume(parser, TOKEN_TYPE_SYMBOL, ")")) {
        compiler_error(parser->compiler, "Expected closing parenthesis");
        printf("Error: Expected closing parenthesis in while statement\n");
        free_ast_node(condition);
        return NULL;
    }
    
    struct ast_node* body = parse_statement(parser);
    
    struct ast_node* while_node = create_while_loop_node(while_token->pos, condition, body);
    return while_node;
}

struct ast_node* parse_if_statement(struct parse_process* parser) {
    printf("Parsing if statement...\n");
    struct token* if_token = consume_token(parser);
    
    if (!check_and_consume(parser, TOKEN_TYPE_SYMBOL, "(")) {
        compiler_error(parser->compiler, "Expected opening parenthesis");
        printf("Error: Expected opening parenthesis in if statement\n");
        return NULL;
    }
    
    struct ast_node* condition = parse_expression(parser);
    
    if (!check_and_consume(parser, TOKEN_TYPE_SYMBOL, ")")) {
        compiler_error(parser->compiler, "Expected closing parenthesis");
        printf("Error: Expected closing parenthesis in if statement\n");
        free_ast_node(condition);
        return NULL;
    }
    
    struct ast_node* then_branch = parse_statement(parser);
    
    struct ast_node* else_branch = NULL;
    if (check_and_consume(parser, TOKEN_TYPE_IDENTIFIER, "else")) {
        else_branch = parse_statement(parser);
    }
    
    struct ast_node* if_node = create_if_stmt_node(if_token->pos, condition, then_branch, else_branch);
    return if_node;
}

struct ast_node* parse_statement(struct parse_process* parser) {
    printf("Parsing statement...\n");
    struct token* token = peek_next_token(parser);
    // printf("peeked successfully\n");
    if(token -> type ==TOKEN_TYPE_SYMBOL ){
        printf("%c\n", token->cval);
    }
    else {
        printf("%s\n", token->sval);
    }
    // printf("%s inside statement funciton\n", token->sval);
    // printf("%d otken type\n", token->type);
    if (token->type == TOKEN_TYPE_KEYWORD) {
        if (strcmp(token->sval, "return") == 0) {
            return parse_return_statement(parser);
        } else if (strcmp(token->sval, "while") == 0) {
            return parse_while_statement(parser);
        } else if (strcmp(token->sval, "if") == 0) {
            return parse_if_statement(parser);
        }else if (strcmp(token->sval, "print") == 0) {
            return parse_print_statement(parser);
        }else {
            // Assume it's a type specifier (for variable declaration or function definition)
            struct token* next_token = vector_get(parser->token_vector, parser->index + 1);
            printf("next_token value: %s\n", next_token->sval);
            if (next_token && next_token->type == TOKEN_TYPE_IDENTIFIER) {
                struct token* after_next = vector_get(parser->token_vector, parser->index + 2);
                printf("after next_token value: %c\n", after_next->cval);
                if (after_next && after_next->type == TOKEN_TYPE_SYMBOL && after_next->cval == ')' || after_next->cval == '(') {
                    return parse_function_definition(parser);
                } else {
                    return parse_declaration(parser);
                }
            }
        }
    } else if (token->type == TOKEN_TYPE_SYMBOL && token->cval == '{') {
        return parse_block(parser);
    } else if (token->type == TOKEN_TYPE_SYMBOL && token->cval == '#') {
        return parse_preprocessor_directive(parser);
    }
    else if (token->type == TOKEN_TYPE_SYMBOL && token->cval == '}') {
        // parser->index++;
        return NULL;
    }
    

    
    printf("Error: Unrecognized statement type\n");
    return NULL;
}
struct ast_node* parse(struct parse_process* parser) {
    printf("Starting parsing process...\n");
    struct ast_node* root = NULL;

    while (parser->index < parser->token_vector_count) {
        struct token* token = (struct token*) vector_get(parser->token_vector, parser->index);
        printf("%s\n inside main block\n", token->sval);
        struct ast_node* stmt = parse_statement(parser);
        if (stmt) {
            if (!root) {
                root = create_ast_node(AST_ROOT, parser->compiler->pos);
                root->root.statements = NULL;
                root->root.stmt_count = 1;
            }
            root->root.statements = realloc(root->root.statements, 
                                              (root->root.stmt_count + 1) * sizeof(struct ast_node*));
            root->root.statements[root->root.stmt_count] = stmt;
            printf("root->statement count: %d\n", root -> root.stmt_count++);
        } else {
            // If no statement is parsed, move to the next token
            consume_token(parser);  // This is important to avoid an infinite loop
        }
        if(parser -> index == parser->token_vector_count -1 )break;
    }
    // printf("root->type %d\n",root->type);
    parser -> root = root;
    printf("root->statement count: %d\n", root -> root.stmt_count++);
    printf("Parsing process completed\n");
    return PARSER_ANALYSIS_ALL_OK;
}
