
#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to generate unique labels
static int label_counter = 0;
static char* generate_label() {
    char* label = malloc(20);
    sprintf(label, "L%d", label_counter++);
    return label;
}

// Helper function to get the size of a type
static int get_type_size(const char* type) {
    if (strcmp(type, "int") == 0) return 8;  // 64-bit integers
    if (strcmp(type, "char") == 0) return 1;
    // Add more types as needed
    return 8;  // Default to 8 bytes for 64-bit architecture
}

void generate_function_prologue(FILE* output, const char* function_name) {
    fprintf(output, "\t.text\n");
    fprintf(output, "\t.globl %s\n", function_name);
    fprintf(output, "\t.type %s, @function\n", function_name);
    fprintf(output, "%s:\n", function_name);
    fprintf(output, "\tpushq %%rbp\n");
    fprintf(output, "\tmovq %%rsp, %%rbp\n");
}

void generate_function_epilogue(FILE* output) {
    fprintf(output, "\tmovq %%rbp, %%rsp\n");
    fprintf(output, "\tpopq %%rbp\n");
    fprintf(output, "\tret\n");
}

void generate_variable_assignment(FILE* output, const char* var_name, int offset, const char* value) {
    fprintf(output, "\tmovq $%s, -%d(%%rbp)\n", value, offset);
}
void generate_print_variable(FILE* output, const char* var_name, int offset) {
    fprintf(output, "\tmovq -%d(%%rbp), %%rdi\n", offset);
    fprintf(output, "\tcall print_int\n");
}
void generate_expression(FILE* output, struct ast_node* node) {
    if (!node) return;

    switch (node->type) {
        case AST_BINARY_OP:
            generate_expression(output, node->binary_op.left);
            fprintf(output, "\tpushq %%rax\n");  // Push left operand result onto the stack
            generate_expression(output, node->binary_op.right);
            fprintf(output, "\tpopq %%rcx\n");   // Pop left operand result from the stack into rcx

            if (strcmp(node->binary_op.operator, "+") == 0) {
                fprintf(output, "\taddq %%rcx, %%rax\n");  // rax = rcx + rax
            } else if (strcmp(node->binary_op.operator, "-") == 0) {
                fprintf(output, "\tsubq %%rax, %%rcx\n");
                fprintf(output, "\tmovq %%rcx, %%rax\n");  // rax = rcx - rax
            } else if (strcmp(node->binary_op.operator, "*") == 0) {
                fprintf(output, "\timulq %%rcx, %%rax\n"); // rax = rcx * rax
            } else if (strcmp(node->binary_op.operator, "/") == 0) {
                fprintf(output, "\tcqo\n");               // Sign-extend rax into rdx
                fprintf(output, "\tidivq %%rcx\n");       // rax = rdx:rax / rcx
            }
            // Add more operators as needed
            break;

        case AST_IDENTIFIER:
            // Load variable value into rax
            fprintf(output, "\tmovq -%d(%%rbp), %%rax\n", get_variable_offset(node->id_literal.value));
            break;

        case AST_NUMBER:
            // Load immediate value into rax
            fprintf(output, "\tmovq $%s, %%rax\n", node->id_literal.value);
            break;

        // Add more cases as needed
    }
}



// // Helper function to get variable offset (you need to implement this)
int get_variable_offset(const char* name) {
    // This function should return the stack offset for the given variable
    // You might need to maintain a symbol table to track variable offsets
    // For now, we'll return a fixed offset as a placeholder
    return 8;
}

void generate_print_int(FILE* output) {
    // Convert integer to string and print
    fprintf(output, "\t# Convert integer to string and print\n");
    fprintf(output, "\tmovq $10, %%r9\n");  // Divisor
    fprintf(output, "\tmovq $0, %%r10\n");  // Digit count
    fprintf(output, "\tmovq %%rax, %%rcx\n");  // Copy number to rcx

    // Handle negative numbers
    fprintf(output, "\ttestq %%rcx, %%rcx\n");
    fprintf(output, "\tjns .Lpositive_%d\n", label_counter);
    fprintf(output, "\tnegq %%rcx\n");
    fprintf(output, "\tmovq $45, (%%rsp)\n");  // ASCII '-'
    fprintf(output, "\tdecq %%rsp\n");
    fprintf(output, "\tincq %%r10\n");

    fprintf(output, ".Lpositive_%d:\n", label_counter);
    // Convert to ASCII and push onto stack
    fprintf(output, ".Lconvert_loop_%d:\n", label_counter);
    fprintf(output, "\txorq %%rdx, %%rdx\n");
    fprintf(output, "\tdivq %%r9\n");
    fprintf(output, "\taddq $48, %%rdx\n");  // Convert to ASCII
    fprintf(output, "\tdecq %%rsp\n");
    fprintf(output, "\tmovb %%dl, (%%rsp)\n");
    fprintf(output, "\tincq %%r10\n");
    fprintf(output, "\ttestq %%rax, %%rax\n");
    fprintf(output, "\tjnz .Lconvert_loop_%d\n", label_counter);

    // Print the number
    fprintf(output, "\tmovq %%r10, %%rdx\n");  // Length
    fprintf(output, "\tmovq %%rsp, %%rsi\n");  // Buffer
    fprintf(output, "\tmovq $1, %%rdi\n");     // File descriptor (stdout)
    fprintf(output, "\tmovq $1, %%rax\n");     // System call number (sys_write)
    fprintf(output, "\tsyscall\n");

    // Print newline
    fprintf(output, "\tmovq $10, (%%rsp)\n");  // ASCII newline
    fprintf(output, "\tmovq $1, %%rdx\n");     // Length
    fprintf(output, "\tmovq %%rsp, %%rsi\n");  // Buffer
    fprintf(output, "\tmovq $1, %%rdi\n");     // File descriptor (stdout)
    fprintf(output, "\tmovq $1, %%rax\n");     // System call number (sys_write)
    fprintf(output, "\tsyscall\n");

    // Restore stack
    fprintf(output, "\taddq %%r10, %%rsp\n");
    fprintf(output, "\tincq %%rsp\n");

    label_counter++;
}
void generate_print_string(FILE* output, const char* string) {
    fprintf(output, "\t# Print string\n");
    fprintf(output, "\tmovq $1, %%rax\n");  // syscall number for sys_write
    fprintf(output, "\tmovq $1, %%rdi\n");  // file descriptor 1 is stdout
    fprintf(output, "\tmovq $.LC%d, %%rsi\n", label_counter);  // address of string to output
    fprintf(output, "\tmovq $%zu, %%rdx\n", strlen(string));  // number of bytes
    fprintf(output, "\tsyscall\n");

    // Print newline
    fprintf(output, "\tmovq $1, %%rax\n");
    fprintf(output, "\tmovq $1, %%rdi\n");
    fprintf(output, "\tmovq $.LC%d, %%rsi\n", label_counter + 1);
    fprintf(output, "\tmovq $1, %%rdx\n");
    fprintf(output, "\tsyscall\n");

    label_counter += 2;
}
void generate_code(FILE* output, struct ast_node* root) {
    if (!root) return;

     static int data_section_added = 0;
    switch (root->type) {
        case AST_FUNCTION_DEFINITION:
            generate_function_prologue(output, root->function_def.name);
            generate_code(output, root->function_def.body);
            generate_function_epilogue(output);
            break;

        case AST_DECLARATION:
            // Allocate space for the variable on the stack
            static int stack_offset = 8;
            fprintf(output, "\tsubq $%d, %%rsp\n", get_type_size(root->declaration.type));
            generate_variable_assignment(output, root->declaration.name, stack_offset, "0");
            stack_offset += get_type_size(root->declaration.type);
            break;

        case AST_BINARY_OP:
            if (strcmp(root->binary_op.operator, "=") == 0) {
                // Variable assignment
                struct ast_node* lhs = root->binary_op.left;
                struct ast_node* rhs = root->binary_op.right;
                generate_expression(output, rhs);  // Compute right-hand side and store in rax
                if (lhs->type == AST_IDENTIFIER) {
                    fprintf(output, "\tmovq %%rax, -%d(%%rbp)\n", get_variable_offset(lhs->id_literal.value));
                }
            } else {
                generate_expression(output, root);  // For other binary operations
            }
            break;

        case AST_PRINT:
            if (root->print.expression->type == AST_STRING) {
                if (!data_section_added) {
                    fprintf(output, "\t.section .rodata\n");
                    data_section_added = 1;
                }
                fprintf(output, ".LC%d:\n", label_counter);
                fprintf(output, "\t.string %s\n", root->print.expression->id_literal.value);
                fprintf(output, ".LC%d:\n", label_counter + 1);
                fprintf(output, "\t.string \"\\n\"\n");
                fprintf(output, "\t.text\n");
                generate_print_string(output, root->print.expression->id_literal.value);
            } else {
                generate_expression(output, root->print.expression);
                generate_print_int(output);  // Assuming we keep this for integer printing
            }
            break;
        case AST_BLOCK:
            for (int i = 0; i < root->block.stmt_count; i++) {
                generate_code(output, root->block.statements[i]);
            }
            break;

        case AST_ROOT:
            fprintf(output, "\t.file \"test.s\"\n");
            fprintf(output, "\t.text\n");
            for (int i = 0; i < root->root.stmt_count; i++) {
                generate_code(output, root->root.statements[i]);
            }
            fprintf(output, "\t.section .note.GNU-stack,\"\",@progbits\n");
        // Add more cases as needed
    }
}