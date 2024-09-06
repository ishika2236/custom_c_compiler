#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int count_local_variables(struct ast_node* node) ;
int get_variable_offset(const char* name);

// Function to generate unique labels
static int label_count = 0;
char* generate_label() {
    char* label = malloc(20);
    sprintf(label, "L%d", label_count++);
    return label;
}

// Function prototypes
void generate_expression(FILE* output, struct ast_node* node);
void generate_statement(FILE* output, struct ast_node* node);

// Generate code for binary operations
void generate_binary_op(FILE* output, struct ast_node* node) {
    generate_expression(output, node->binary_op.left);
    fprintf(output, "    push %%rax\n");
    generate_expression(output, node->binary_op.right);
    fprintf(output, "    pop %%rcx\n");

    if (strcmp(node->binary_op.operator, "+") == 0) {
        fprintf(output, "    addq %%rcx, %%rax\n");
    } else if (strcmp(node->binary_op.operator, "-") == 0) {
        fprintf(output, "    subq %%rax, %%rcx\n");
        fprintf(output, "    movq %%rcx, %%rax\n");
    } else if (strcmp(node->binary_op.operator, "*") == 0) {
        fprintf(output, "    imulq %%rcx\n");
    } else if (strcmp(node->binary_op.operator, "/") == 0) {
        fprintf(output, "    xchgq %%rax, %%rcx\n");
        fprintf(output, "    cqto\n");
        fprintf(output, "    idivq %%rcx\n");
    }
    // Add more operations as needed
}

// Generate code for expressions
void generate_expression(FILE* output, struct ast_node* node) {
    switch (node->type) {
        case AST_NUMBER:
            fprintf(output, "    movq $%s, %%rax\n", node->id_literal.value);
            break;
        case AST_IDENTIFIER:
            // Assume variables are stored on the stack
            fprintf(output, "    movq -%d(%%rbp), %%rax\n", 8 * get_variable_offset(node->id_literal.value));
            break;
        case AST_BINARY_OP:
            generate_binary_op(output, node);
            break;
        // Add more expression types as needed
        default:
            fprintf(stderr, "Unsupported expression type\n");
            exit(1);
    }
}

// Generate code for print statements
void generate_print_statement(FILE* output, struct ast_node* node) {
    generate_expression(output, node->print.expression);
    fprintf(output, "    movq %%rax, %%rsi\n");
    fprintf(output, "    movq $print_format, %%rdi\n");
    fprintf(output, "    xorq %%rax, %%rax\n");
    fprintf(output, "    call printf\n");
}

// Generate code for variable declarations
void generate_declaration(FILE* output, struct ast_node* node) {
    if (node->declaration.initial_value) {
        generate_expression(output, node->declaration.initial_value);
    } else {
        fprintf(output, "    xorq %%rax, %%rax\n");
    }
    fprintf(output, "    pushq %%rax\n");
}

// Generate code for return statements
void generate_return_statement(FILE* output, struct ast_node* node) {
    if (node->return_stmt.value) {
        generate_expression(output, node->return_stmt.value);
    }
    fprintf(output, "    leave\n");
    fprintf(output, "    ret\n");
}

// Generate code for if statements
void generate_if_statement(FILE* output, struct ast_node* node) {
    char* else_label = generate_label();
    char* end_label = generate_label();

    generate_expression(output, node->if_stmt.condition);
    fprintf(output, "    cmp $0, %%rax\n");
    fprintf(output, "    je %s\n", else_label);

    generate_statement(output, node->if_stmt.true_body);
    fprintf(output, "    jmp %s\n", end_label);

    fprintf(output, "%s:\n", else_label);
    if (node->if_stmt.false_body) {
        generate_statement(output, node->if_stmt.false_body);
    }

    fprintf(output, "%s:\n", end_label);

    free(else_label);
    free(end_label);
}

// Generate code for while loops
void generate_while_loop(FILE* output, struct ast_node* node) {
    char* start_label = generate_label();
    char* end_label = generate_label();

    fprintf(output, "%s:\n", start_label);
    generate_expression(output, node->while_loop.condition);
    fprintf(output, "    cmp $0, %%rax\n");
    fprintf(output, "    je %s\n", end_label);

    generate_statement(output, node->while_loop.body);
    fprintf(output, "    jmp %s\n", start_label);

    fprintf(output, "%s:\n", end_label);

    free(start_label);
    free(end_label);
}

// Generate code for statements
void generate_statement(FILE* output, struct ast_node* node) {
    switch (node->type) {
        case AST_PRINT:
            generate_print_statement(output, node);
            break;
        case AST_DECLARATION:
            generate_declaration(output, node);
            break;
        case AST_RETURN:
            generate_return_statement(output, node);
            break;
        case AST_IF_STMT:
            generate_if_statement(output, node);
            break;
        case AST_WHILE_LOOP:
            generate_while_loop(output, node);
            break;
        case AST_BLOCK:
            for (int i = 0; i < node->block.stmt_count; i++) {
                generate_statement(output, node->block.statements[i]);
            }
            break;
        // Add more statement types as needed
        default:
            fprintf(stderr, "Unsupported statement type\n");
            exit(1);
    }
}

// Generate code for function definitions
void generate_function(FILE* output, struct ast_node* node) {
    fprintf(output, ".globl %s\n", node->function_def.name);
    fprintf(output, "%s:\n", node->function_def.name);
    fprintf(output, "    pushq %%rbp\n");
    fprintf(output, "    movq %%rsp, %%rbp\n");

    // Allocate space for local variables
    int local_var_count = count_local_variables(node->function_def.body);
    if (local_var_count > 0) {
        fprintf(output, "    subq $%d, %%rsp\n", 8 * local_var_count);
    }

    generate_statement(output, node->function_def.body);

    // If there's no explicit return, add one
    if (node->function_def.body->type != AST_RETURN) {
        fprintf(output, "    leave\n");
        fprintf(output, "    ret\n");
    }
}

// Main code generation function
void generate_code(FILE* output, struct ast_node* root) {
    
    fprintf(output, ".data\n");
    fprintf(output, "print_format: .asciz \"%%d\\n\"\n");
    fprintf(output, ".text\n");
    

    for (int i = 0; i < root->root.stmt_count; i++) {
        
        struct ast_node* node = root->root.statements[i];
        if (node->type == AST_FUNCTION_DEFINITION) {
            // printf("hello from generator\n");
            generate_function(output, node);
        }
        else if(node->type == AST_DECLARATION){
                // Handle global variables
                fprintf(output, ".globl %s\n", node->declaration.name);
                fprintf(output, "%s:\n", node->declaration.name);
                if (node->declaration.initial_value) {
                    fprintf(output, "    .quad %s\n", node->declaration.initial_value->id_literal.value);
                } else {
                    fprintf(output, "    .quad 0\n");
                }
                break;
        }
        // Handle global variables or other top-level constructs if needed
    }
}

// Helper function to count local variables in a function body
int count_local_variables(struct ast_node* node) {
    int count = 0;
    if (node->type == AST_BLOCK) {
        for (int i = 0; i < node->block.stmt_count; i++) {
            if (node->block.statements[i]->type == AST_DECLARATION) {
                count++;
            }
        }
    }
    return count;
}

// Helper function to get variable offset (you'll need to implement this)
int get_variable_offset(const char* name) {
    // This function should return the stack offset for a given variable
    // You'll need to implement a symbol table to track this
    // For now, we'll just return a placeholder value
    return 8;
}