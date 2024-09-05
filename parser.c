#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include "./helpers/vector.h"
#include <string.h>
void syntax_error(const char* message, struct token* token);
struct ast_node* parse_type(struct parse_process* parser);
struct ast_node* parse_condition(struct parse_process* parser);
struct ast_node* parse_control_structure(struct parse_process* parser);
struct ast_node* parse_statement(struct parse_process* parser);
struct ast_node* parse_function_body(struct parse_process* parser);
struct ast_node* parse_return_statement(struct parse_process* parser);
int is_declaration(struct token* token);
struct ast_node* parse_while_loop(struct parse_process* parser);
struct ast_node* parse_declaration(struct parse_process* parser);
struct ast_node* parse_expression(struct parse_process* parser);
struct ast_node* parse_statement(struct parse_process* parser);
struct ast_node* parse_program(struct parse_process* parser);
struct ast_node* parse_primary_expression(struct parse_process* parser);
struct ast_node* parse_expression_tail(struct parse_process* parser, struct ast_node* left);


void syntax_error(const char* message, struct token* token) {
    fprintf(stderr, "Syntax error: %s at line %d\n", message, token->pos.line);
    exit(EXIT_FAILURE);
}
struct ast_node* parse_primary_expression(struct parse_process* parser) {
    struct token* token = get_next_token(parser);

    if (token->type == TOKEN_TYPE_IDENTIFIER) {
        return create_ast_node_with_value(AST_IDENTIFIER, token->sval);
    } else if (token->type == TOKEN_TYPE_NUMBER) {
        return create_ast_node_with_value(AST_LITERAL, token->sval);
    } else if (token->type == TOKEN_TYPE_SYMBOL && token->cval == '(') {
        struct ast_node* expr_node = parse_expression(parser);
        if (expr_node == NULL) {
            syntax_error("Failed to parse expression inside parentheses", NULL);
        }

        struct token* close_paren = get_next_token(parser);
        if (close_paren->type != TOKEN_TYPE_SYMBOL || close_paren->cval != ')') {
            syntax_error("Expected ')' after expression", close_paren);
        }

        return expr_node;
    } else {
        syntax_error("Expected primary expression", token);
        return NULL;
    }
}

// Parse an expression with precedence and operators
struct ast_node* parse_expression_tail(struct parse_process* parser, struct ast_node* left) {
    struct token* token = peek_next_token(parser);

    while (token->type == TOKEN_TYPE_OPERATOR) {
        // Handle different operators and their precedence
        if (strcmp(token->sval, "+") == 0 || strcmp(token->sval, "-") == 0) {
            get_next_token(parser); // Consume operator
            struct ast_node* right = parse_primary_expression(parser);
            if (right == NULL) {
                syntax_error("Failed to parse right-hand side of expression", peek_next_token(parser));
            }

            struct ast_node* op_node = create_ast_node_with_value(AST_OPERATOR, token->sval);
            struct ast_node* expr_node = create_ast_node(AST_BINARY_OP);
            add_child(expr_node, left);
            add_child(expr_node, op_node);
            add_child(expr_node, right);

            left = expr_node; // Continue parsing the rest of the expression
        } else if (strcmp(token->sval, "*") == 0 || strcmp(token->sval, "/") == 0) {
            get_next_token(parser); // Consume operator
            struct ast_node* right = parse_primary_expression(parser);
            if (right == NULL) {
                syntax_error("Failed to parse right-hand side of expression", peek_next_token(parser));
            }

            struct ast_node* op_node = create_ast_node_with_value(AST_OPERATOR, token->sval);
            struct ast_node* expr_node = create_ast_node(AST_BINARY_OP);
            add_child(expr_node, left);
            add_child(expr_node, op_node);
            add_child(expr_node, right);

            left = expr_node; // Continue parsing the rest of the expression
        } else {
            break; // No more operators, end of expression
        }

        token = peek_next_token(parser);
    }

    return left;
}

// Parse an expression
struct ast_node* parse_expression(struct parse_process* parser) {
    struct ast_node* left = parse_primary_expression(parser);
    if (left == NULL) {
        return NULL;
    }

    return parse_expression_tail(parser, left);
}

struct ast_node* parse_type(struct parse_process* parser) {
    struct token* current_token = get_next_token(parser);
    // printf("current token value %s, current token type : %d",current_token->sval, current_token->type );
    if (current_token->type == TOKEN_TYPE_KEYWORD) {
        // printf(" token checking in parse type %s\n",current_token -> sval);
        struct ast_node* type_node = create_ast_node_with_value(AST_IDENTIFIER, current_token->sval);
        return type_node;
    } else {
        syntax_error("Expected a type specifier", current_token);
        return NULL;
    }

}
struct ast_node* parse_condition(struct parse_process* parser) {
    // Assuming '(' has already been consumed
    struct ast_node* condition_node = parse_expression(parser);
    if (condition_node == NULL) {
        syntax_error("Failed to parse condition", NULL);
        return NULL;
    }

    struct token* token = vector_get(parser->token_vector, parser->index);
    if (token->type == TOKEN_TYPE_SYMBOL && token->cval == ')') {
        parser->index++; // Move past the ')'
        return condition_node;
    } else {
        syntax_error("Expected ')' after condition", token);
        return NULL;
    }
}
struct ast_node* parse_control_structure(struct parse_process* parser) {
    struct token* token = vector_get(parser->token_vector, parser->index);
    struct ast_node* control_node = NULL;

    if (strcmp(token->sval, "if") == 0) {
        parser->index++; // Move past 'if'
        
        if (!parse_process_match(parser, TOKEN_TYPE_SYMBOL, "(")) {
            syntax_error("Expected '(' after 'if'", peek_next_token(parser));
            return NULL;
        }

        struct ast_node* condition = parse_condition(parser);
        if (condition == NULL) return NULL;

        if (!parse_process_match(parser, TOKEN_TYPE_SYMBOL, "{")) {
            syntax_error("Expected '{' after condition", peek_next_token(parser));
            return NULL;
        }

        struct ast_node* body = parse_statement(parser);
        if (body == NULL) return NULL;

        control_node = create_ast_node(AST_IF);
        add_child(control_node, condition);
        add_child(control_node, body);

        // Parse optional 'else' block
        token = vector_get(parser->token_vector, parser->index);
        if (strcmp(token->sval, "else") == 0) {
            parser->index++; // Move past 'else'
            token = vector_get(parser->token_vector, parser->index);
            if (token -> type != TOKEN_TYPE_SYMBOL && token->cval != '{') {
                syntax_error("Expected '{' after 'else'", peek_next_token(parser));
                return NULL;
            }

            struct ast_node* else_body = parse_statement(parser);
            if (else_body == NULL) return NULL;

            struct ast_node* else_node = create_ast_node(AST_ELSE);
            add_child(else_node, else_body);
            add_child(control_node, else_node);
        }

    } else if (strcmp(token->sval, "while") == 0) {
        // Similar parsing logic for 'while'
        // Ensure to parse the condition and body
    } else if (strcmp(token->sval, "for") == 0) {
        // Similar parsing logic for 'for'
        // Parse initialization, condition, increment, and body
    } else {
        syntax_error("Unexpected control structure", token);
        return NULL;
    }

    return control_node;
}


struct ast_node* parse_statement(struct parse_process* parser) {
    while(peek_next_token(parser)-> type == TOKEN_TYPE_NEWLINE){
        get_next_token(parser);
    }
    struct token* token = (struct token*) vector_get(parser->token_vector, parser->index);
    printf("token value in parse_statement %s\n", token->sval);
    if (strcmp(token->sval, "if") == 0 || strcmp(token->sval, "while") == 0 || strcmp(token->sval, "for") == 0) {
        return parse_control_structure(parser);
    }
    else if (strcmp(token->sval, "return") == 0) {
        printf("i am here\n");
        return parse_return_statement(parser);
    }
    else if (is_declaration(token)) {
        return parse_declaration(parser);
    }
    else {
        return parse_expression(parser);
    }
}
struct ast_node* parse_function_body(struct parse_process* parser)
{
    
    struct token* current_token = get_next_token(parser);
    // printf("current token in function block : %c\n ", current_token->cval);
    if(current_token == NULL || current_token -> type != TOKEN_TYPE_SYMBOL && current_token -> cval !='{')
    {
        syntax_error("expected '{' at the beginning of function body \n", current_token);
        return NULL;
    }

    struct ast_node* body_node = create_ast_node(AST_BLOCK);
    while (peek_next_token(parser)->type != TOKEN_TYPE_SYMBOL && peek_next_token(parser)->cval != '}') {
        struct ast_node* statement_node = parse_statement(parser);
        if (statement_node != NULL) {
            add_child(body_node, statement_node);
        }
        else {
            syntax_error("Statement parsing returned null", NULL);
        }
    }


        parser->index++;  
    // get_next_token(parser);

    return body_node;

}
struct ast_node* parse_return_statement(struct parse_process* parser) {
    struct token* return_token = vector_get(parser->token_vector, parser->index);
    
    if (return_token == NULL || strcmp(return_token->sval, "return") != 0) {
        syntax_error("Expected 'return'", return_token);
        return NULL;
    }

    parser->index++; // Move past 'return'
    struct ast_node* return_node = create_ast_node(AST_RETURN);
    struct ast_node* return_value_node = NULL;
   
    struct token* next_token = vector_get(parser->token_vector, parser->index);
    printf("%d in return function\n", next_token->type);
    if (next_token->type != TOKEN_TYPE_SYMBOL || next_token->cval != ';') {
        return_value_node = parse_expression(parser);
        if (return_value_node == NULL) {
            syntax_error("Failed to parse return value", peek_next_token(parser));
            return NULL;
        }
    }
    else{
        printf("encountered error  \n  token_value == %d\n", next_token->type);
    }

    

    return return_node;
}

int is_declaration(struct token* token) {
    if (token == NULL) {
        return 0;
    }

    // Check if the token is one of the data type keywords used in declarations
    return strcmp(token->sval, "int") == 0 ||
           strcmp(token->sval, "float") == 0 ||
           strcmp(token->sval, "char") == 0 ||
           strcmp(token->sval, "double") == 0 ||
           strcmp(token->sval, "void") == 0;
}
struct ast_node* parse_while_loop(struct parse_process* parser) {
    struct token* token = vector_get(parser->token_vector, parser->index);
    if (strcmp(token->sval, "while") != 0) {
        syntax_error("Expected 'while'", token);
        return NULL;
    }

    parser->index++; // Move past 'while'

    if (!parse_process_match(parser, TOKEN_TYPE_SYMBOL, "(")) {
        syntax_error("Expected '(' after 'while'", peek_next_token(parser));
        return NULL;
    }

    struct ast_node* condition = parse_condition(parser);
    if (condition == NULL) return NULL;

    if (!parse_process_match(parser, TOKEN_TYPE_SYMBOL, ")")) {
        syntax_error("Expected ')' after condition", peek_next_token(parser));
        return NULL;
    }

    if (!parse_process_match(parser, TOKEN_TYPE_SYMBOL, "{")) {
        syntax_error("Expected '{' after 'while' condition", peek_next_token(parser));
        return NULL;
    }

    struct ast_node* body = parse_statement(parser);
    if (body == NULL) return NULL;

    struct ast_node* while_node = create_ast_node(AST_WHILE);
    add_child(while_node, condition);
    add_child(while_node, body);

    return while_node;
}
struct ast_node* parse_for_loop(struct parse_process* parser) {
    struct token* token = vector_get(parser->token_vector, parser->index);
    if (strcmp(token->sval, "for") != 0) {
        syntax_error("Expected 'for'", token);
        return NULL;
    }

    parser->index++; // Move past 'for'

    if (!parse_process_match(parser, TOKEN_TYPE_SYMBOL, "(")) {
        syntax_error("Expected '(' after 'for'", peek_next_token(parser));
        return NULL;
    }

    // Parse initialization
    struct ast_node* init = parse_expression(parser);
    if (init == NULL) {
        syntax_error("Failed to parse 'for' initialization", peek_next_token(parser));
        return NULL;
    }

    // Parse condition
    struct ast_node* condition = parse_expression(parser);
    if (condition == NULL) {
        syntax_error("Failed to parse 'for' condition", peek_next_token(parser));
        return NULL;
    }

    // Parse increment
    struct ast_node* increment = parse_expression(parser);
    if (increment == NULL) {
        syntax_error("Failed to parse 'for' increment", peek_next_token(parser));
        return NULL;
    }

    if (!parse_process_match(parser, TOKEN_TYPE_SYMBOL, ")")) {
        syntax_error("Expected ')' after 'for' expression", peek_next_token(parser));
        return NULL;
    }

    if (!parse_process_match(parser, TOKEN_TYPE_SYMBOL, "{")) {
        syntax_error("Expected '{' after 'for' loop", peek_next_token(parser));
        return NULL;
    }

    struct ast_node* body = parse_statement(parser);
    if (body == NULL) return NULL;

    struct ast_node* for_node = create_ast_node(AST_FOR);
    add_child(for_node, init);
    add_child(for_node, condition);
    add_child(for_node, increment);
    add_child(for_node, body);

    return for_node;
}


struct ast_node* parse_declaration(struct parse_process* parser)
{
    // printf("hello i am running \n");
    struct token* current_token = peek_next_token(parser);
    if (current_token == NULL) {
        syntax_error("Failed to peek next token", NULL);
        return NULL;
    }

    // Skipping comments and whitespace tokens
    while (current_token != NULL && 
           (current_token->type == TOKEN_TYPE_COMMENT || current_token-> type == TOKEN_TYPE_NEWLINE )) {
        current_token = get_next_token(parser);
    }

    // Check for valid type keywords
    // struct token* token = peek_next_token(parser);
    printf("%s\n", current_token->sval);
    
    if (is_declaration(current_token)) {

        struct ast_node* type_node = parse_type(parser);
       
        if (type_node == NULL) return NULL;
        struct token* identifier_token = get_next_token(parser);
        if (identifier_token == NULL || identifier_token->type != TOKEN_TYPE_IDENTIFIER) {
            syntax_error("Expected identifier after type", identifier_token);
            return NULL;
        }

        struct ast_node* identifier_node = create_ast_node_with_value(AST_IDENTIFIER, identifier_token->sval);
        if (identifier_node == NULL) return NULL;

        struct token* token3 = get_next_token(parser);
        if(token3 -> type == TOKEN_TYPE_OPERATOR){
            printf("reading operator %s\n", token3 ->sval);
        }
         printf("type_node -> value %s  \n",type_node->value );
        printf("identifier_node -> value %s  \n",identifier_node->value );
        
        if (token3 -> type == TOKEN_TYPE_SYMBOL && token3 -> cval =='(') {
            // get_next_token(parser);
            struct ast_node* func_decl_node = create_ast_node(AST_FUNCTION);
            if (func_decl_node == NULL) return NULL;

            add_child(func_decl_node, type_node);
            add_child(func_decl_node, identifier_node);
            
            // printf("%s\n", peek_next_token(parser)->sval);
            while (peek_next_token(parser)-> type != TOKEN_TYPE_SYMBOL && peek_next_token(parser)->cval != ')') {
                struct ast_node* param_type = parse_type(parser);
                if (param_type == NULL) return NULL;
                // printf("error encountered pehle hi\n");
                struct token* param_name_token = get_next_token(parser);
                if (param_name_token == NULL || param_name_token->type != TOKEN_TYPE_IDENTIFIER) {
                    syntax_error("Expected parameter name", param_name_token);
                    return NULL;
                }

                struct ast_node* param_name = create_ast_node_with_value(AST_IDENTIFIER, param_name_token->sval);
                if (param_name == NULL) return NULL;

                add_child(func_decl_node, param_type);
                add_child(func_decl_node, param_name);
                printf("param_type -> value %s  \n",type_node->value );
                printf("param_name -> value %s  \n",param_name->value );
                if (peek_next_token(parser)-> type == TOKEN_TYPE_SYMBOL && peek_next_token(parser)->cval == ',') {
                    get_next_token(parser);
                }
                else if (peek_next_token(parser)-> type == TOKEN_TYPE_SYMBOL && peek_next_token(parser)->cval == ')')
                {
                    break;
                }
                else {
                    // parser -> index++;
                    while(peek_next_token(parser)->type == TOKEN_TYPE_NEWLINE ){
                        get_next_token(parser);
                    }
                    
                }
                // printf("declarations: %s\n", token->sval);
                
                struct token* tokenbreak = get_next_token(parser); 
                if(tokenbreak -> type == TOKEN_TYPE_SYMBOL && tokenbreak -> cval == ')' )
                {
                    break;
                }

            }
            printf("i came out of loop\n");
            
            get_next_token(parser);
            while(peek_next_token(parser)->type == TOKEN_TYPE_NEWLINE ){
                get_next_token(parser);
            }
            struct token* token = peek_next_token(parser);
            printf("%c\n",token->cval);
            // printf("%s\n", func_decl_node->value);
            
             struct ast_node* function_body = parse_function_body(parser);
             
                if (function_body == NULL) {
                    syntax_error("Expected function body", token3);
                    return NULL;
                }

            add_child(func_decl_node, function_body);
            return func_decl_node;
            return func_decl_node;

        } 
        else if (token3 -> type == TOKEN_TYPE_OPERATOR || token3 -> type == TOKEN_TYPE_SYMBOL) {
            printf("i am here\n");
             struct ast_node* var_decl_node = create_ast_node(AST_VARIABLE);
            if (var_decl_node == NULL) {
                printf("error in creating node\n");
                return NULL;
            }
            
             add_child(var_decl_node, type_node);
            add_child(var_decl_node, identifier_node);
            printf("children added successfully\n");
            printf("token 3 type == %c\n", token3->cval);
            while (peek_next_token(parser) != NULL && 
            (peek_next_token(parser)->type == TOKEN_TYPE_NEWLINE || 
                peek_next_token(parser)->type == TOKEN_TYPE_COMMENT)) {
                get_next_token(parser);
            }

            // Update token3 to the next valid token after skipping
            // token3 = peek_next_token(parser);

            printf("token 3 type == %c\n", token3->cval);  
            return var_decl_node;
            printf("i am stupid and annoying\n");
            // get_next_token(parser);
            struct token* token = get_next_token(parser);
            printf("token value in declaration : %s", token->sval);
            
            // printf("%s\n", var_decl_node->value);
            return var_decl_node;
        }
        else{
            printf("i am stupid\n");
        }
    }
    printf("invalid debug: %s\n", current_token->sval);
    syntax_error("Invalid declaration", current_token);
    return NULL;
}
struct ast_node* parser_return_statement(struct parse_process* parser)
{
    struct ast_node* node = create_ast_node(AST_RETURN);

    parser -> index++;
    return node;

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
