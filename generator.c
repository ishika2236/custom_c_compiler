
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
struct symbol {
    char* name;
    int offset;
};

struct symbol symbol_table[100];  // Adjust size as needed
int symbol_count = 0;

void add_symbol(const char* name, int offset) {
    symbol_table[symbol_count].name = strdup(name);
    symbol_table[symbol_count].offset = offset;
    symbol_count++;
}

int get_variable_offset(const char* name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return symbol_table[i].offset;
        }
    }
    fprintf(stderr, "Error: Variable %s not found\n", name);
    exit(1);
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
            fprintf(output, "\tpushq %%rax\n");
            generate_expression(output, node->binary_op.right);
            fprintf(output, "\tpopq %%rcx\n");

            if (strcmp(node->binary_op.operator, "+") == 0) {
                fprintf(output, "\taddq %%rcx, %%rax\n");
            } else if (strcmp(node->binary_op.operator, "-") == 0) {
                fprintf(output, "\tsubq %%rax, %%rcx\n");
                fprintf(output, "\tmovq %%rcx, %%rax\n");
            } else if (strcmp(node->binary_op.operator, "*") == 0) {
                fprintf(output, "\timulq %%rcx, %%rax\n");
            } else if (strcmp(node->binary_op.operator, "/") == 0) {
                fprintf(output, "\tcqo\n");
                fprintf(output, "\tidivq %%rcx\n");
            } else if (strcmp(node->binary_op.operator, ">") == 0) {
                fprintf(output, "\tcmpq %%rax, %%rcx\n");
                fprintf(output, "\tsetg %%al\n");
                fprintf(output, "\tmovzbq %%al, %%rax\n");
            } else if (strcmp(node->binary_op.operator, "<") == 0) {
                fprintf(output, "\tcmpq %%rax, %%rcx\n");
                fprintf(output, "\tsetl %%al\n");
                fprintf(output, "\tmovzbq %%al, %%rax\n");
            } else if (strcmp(node->binary_op.operator, ">=") == 0) {
                fprintf(output, "\tcmpq %%rax, %%rcx\n");
                fprintf(output, "\tsetge %%al\n");
                fprintf(output, "\tmovzbq %%al, %%rax\n");
            } else if (strcmp(node->binary_op.operator, "<=") == 0) {
                fprintf(output, "\tcmpq %%rax, %%rcx\n");
                fprintf(output, "\tsetle %%al\n");
                fprintf(output, "\tmovzbq %%al, %%rax\n");
            } else if (strcmp(node->binary_op.operator, "==") == 0) {
                fprintf(output, "\tcmpq %%rax, %%rcx\n");
                fprintf(output, "\tsete %%al\n");
                fprintf(output, "\tmovzbq %%al, %%rax\n");
            } else if (strcmp(node->binary_op.operator, "!=") == 0) {
                fprintf(output, "\tcmpq %%rax, %%rcx\n");
                fprintf(output, "\tsetne %%al\n");
                fprintf(output, "\tmovzbq %%al, %%rax\n");
            } else if (strcmp(node->binary_op.operator, "+=") == 0 ||
                       strcmp(node->binary_op.operator, "-=") == 0 ||
                       strcmp(node->binary_op.operator, "*=") == 0 ||
                       strcmp(node->binary_op.operator, "/=") == 0) {
                // Compound assignment operators
                char op = node->binary_op.operator[0];
                int offset = get_variable_offset(node->binary_op.left->id_literal.value);
                fprintf(output, "\tmovq -%d(%%rbp), %%rcx\n", offset);
                switch (op) {
                    case '+': fprintf(output, "\taddq %%rax, %%rcx\n"); break;
                    case '-': fprintf(output, "\tsubq %%rax, %%rcx\n"); break;
                    case '*': fprintf(output, "\timulq %%rax, %%rcx\n"); break;
                    case '/': 
                        fprintf(output, "\txchgq %%rax, %%rcx\n");
                        fprintf(output, "\tcqo\n");
                        fprintf(output, "\tidivq %%rcx\n");
                        fprintf(output, "\tmovq %%rax, %%rcx\n");
                        break;
                }
                fprintf(output, "\tmovq %%rcx, -%d(%%rbp)\n", offset);
                fprintf(output, "\tmovq %%rcx, %%rax\n");
            }
            break;

        case AST_UNARY_OP:
            if (strcmp(node->unary_op.operator, "++") == 0 ||
                strcmp(node->unary_op.operator, "--") == 0) {
                int offset = get_variable_offset(node->unary_op.operand->id_literal.value);
                fprintf(output, "\tmovq -%d(%%rbp), %%rax\n", offset);
                if (strcmp(node->unary_op.operator, "++") == 0) {
                    fprintf(output, "\tincq %%rax\n");
                } else {
                    fprintf(output, "\tdecq %%rax\n");
                }
                fprintf(output, "\tmovq %%rax, -%d(%%rbp)\n", offset);
            }
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
        case AST_IDENTIFIER:
            fprintf(output, "\tmovq -%d(%%rbp), %%rax\n", get_variable_offset(root->id_literal.value));
            break;
        case AST_FUNCTION_DEFINITION:
            generate_function_prologue(output, root->function_def.name);
            generate_code(output, root->function_def.body);
            generate_function_epilogue(output);
            break;

        case AST_DECLARATION:
            static int stack_offset = 8;
            fprintf(output, "\tsubq $%d, %%rsp\n", get_type_size(root->declaration.type));
            add_symbol(root->declaration.name, stack_offset);
            if (root->declaration.initial_value) {
                generate_expression(output, root->declaration.initial_value);
                fprintf(output, "\tmovq %%rax, -%d(%%rbp)\n", stack_offset);
            } else {
                generate_variable_assignment(output, root->declaration.name, stack_offset, "0");
            }
            stack_offset += get_type_size(root->declaration.type);
            break;
        case AST_BINARY_OP:
            if (strcmp(root->binary_op.operator, "=") == 0) {
                struct ast_node* lhs = root->binary_op.left;
                struct ast_node* rhs = root->binary_op.right;
                generate_expression(output, rhs);  // Compute right-hand side and store in rax
                if (lhs->type == AST_IDENTIFIER) {
                    fprintf(output, "\tmovq %%rax, -%d(%%rbp)\n", get_variable_offset(lhs->id_literal.value));
                }
            } if (strcmp(root->binary_op.operator, "==") == 0) {
                fprintf(output, "\tcmpq %%rax, %%rcx\n");
                fprintf(output, "\tsete %%al\n");
                fprintf(output, "\tmovzbq %%al, %%rax\n");
            }
            else {
                generate_expression(output, root);
            }
            break;

        case AST_PRINT:
            
            generate_expression(output, root->print.expression);
            generate_print_int(output);
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
            break;
        case AST_IF_STMT:
                
                char* else_label = generate_label();
                char* end_if_label = generate_label();

                // Generate code for the condition
                generate_expression(output, root->if_stmt.condition);
                
                // Compare the result with 0
                fprintf(output, "\tcmpq $0, %%rax\n");
                fprintf(output, "\tje %s\n", else_label);

                // Generate code for the then branch
                generate_code(output, root->if_stmt.true_body);
                fprintf(output, "\tjmp %s\n", end_if_label);
               
                // Else branch (if it exists)
                fprintf(output, "%s:\n", else_label);
                if (root->if_stmt.false_body) {
                    generate_code(output, root->if_stmt.false_body);
                }

                // End of if statement
                fprintf(output, "%s:\n", end_if_label);
                
                free(else_label);
                free(end_if_label);
            
            break;
        case AST_WHILE:
            {
                char* start_label = generate_label();
                char* end_label = generate_label();

                // Start of while loop
                fprintf(output, "%s:\n", start_label);

                // Generate code for the condition
                generate_expression(output, root->while_loop.condition);

                // Compare the result with 0
                fprintf(output, "\tcmpq $0, %%rax\n");
                fprintf(output, "\tje %s\n", end_label);

                // Generate code for the loop body
                generate_code(output, root->while_loop.body);

                // Jump back to the start of the loop
                fprintf(output, "\tjmp %s\n", start_label);

                // End of while loop
                fprintf(output, "%s:\n", end_label);

                free(start_label);
                free(end_label);
            }
            break;
        case AST_RETURN:
            // Generate code for the return expression (if any)
            if (root->return_stmt.value) {
                generate_expression(output, root->return_stmt.value);
            }

            // Function epilogue
            fprintf(output, "\tmovq %%rbp, %%rsp\n");
            fprintf(output, "\tpopq %%rbp\n");
            fprintf(output, "\tret\n");
            break;
    }
}