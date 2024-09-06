# C Compiler Project

This project is a simple C compiler implemented in C. It compiles a subset of the C language to x86-64 assembly.

## Features

- Lexical analysis (tokenization)
- Parsing (AST generation)
- Code generation (x86-64 assembly)
- Support for basic C constructs:
  - Variable declarations
  - Function definitions
  - Basic arithmetic operations
  - Print statements
  - If-else statements
  - While loops

## Prerequisites

- GCC (GNU Compiler Collection)
- Make

## Program Structure

The compiler is organized into several modules:

```
.
├── Makefile
|
│── compiler.h
│── compiler.c
│── lexer.c
│── parser.c
│── generator.c
│── helpers
│    └── vector.c
|    └── buffer.c
│── main.c
|
│── test.c
└── README.md
```

- `compiler.h`: Main header file with structure definitions and function declarations.
- `compiler.c`: Implementation of core compiler functions.
- `lexer.c`: Tokenization of input source code.
- `parser.c`: Parsing tokens into an Abstract Syntax Tree (AST).
- `generator.c`: Generation of x86-64 assembly code from the AST.
- `helpers/vector.c`: Implementation of a dynamic array used throughout the compiler.
- `main.c`: Entry point of the compiler.

## Implementation Details

### Lexical Analysis (Lexer)

The lexer (`lexer.c`) breaks down the input source code into a series of tokens. It recognizes:

- Keywords (e.g., `int`, `return`, `if`, `while`)
- Identifiers
- Numbers (integers)
- Operators
- Symbols (parentheses, braces, semicolons)
- String literals

Each token is represented by a `struct token` which includes the token type, value, and position in the source code.

### Parsing (Parser)

The parser (`parser.c`) takes the stream of tokens from the lexer and constructs an Abstract Syntax Tree (AST). It implements a recursive descent parser for the supported C grammar. The main parsing functions include:

- `parse_expression`: Handles arithmetic expressions
- `parse_statement`: Parses various statement types (declarations, if-else, while, return, etc.)
- `parse_function_definition`: Parses function definitions
- `parse_block`: Handles code blocks (compound statements)

The AST is represented using `struct ast_node`, with different node types for various language constructs.

### Code Generation

The code generator (`generator.c`) traverses the AST and produces x86-64 assembly code. It includes functions for:

- Generating function prologues and epilogues
- Handling variable assignments and arithmetic operations
- Implementing control structures (if-else, while loops)
The generated assembly uses the System V AMD64 ABI calling convention.

## Building the Compiler

To build the compiler, follow these steps:

1. Open a terminal in the project root directory.
2. Run the following command:

   ```
   make
   ```

   This will compile the compiler source code and create an executable named `compiler`.

## Cleaning the Build

To clean the build artifacts, run:

```
make clean
```

This will remove all object files and the `compiler` executable.

## Compiling and Running a C Program

1. Write your C program in a file (e.g., `test.c`).
2. Compile your C program to assembly:

   ```
   ./main
   ```

   This will generate an assembly file named `test.s`.

3. Assemble the generated assembly file:

   ```
   gcc test.s -c -o test.o
   ```

4. Link the object file to create an executable:

   ```
   gcc test.o -o test
   ```

5. Run the compiled program:

   ```
   ./test
   ```

## Example

Here's an example of how to compile and run a simple C program:

1. Create a file named `test.c` with the following content:

   ```c
   int main() {
       int x = 5;
       print(x);
       return 0;
   }
   ```

2. Compile and run the program:

   ```
   ./main
   gcc test.s -c -o test.o
   gcc test.o -o test
   ./test
   ```

   This should output:

   ```
   5
   ```

## Limitations

This compiler is a simplified implementation and does not support the full C language specification. Some limitations include:

- Limited type support (mainly integers)
- No support for structs, unions, or enums
- Limited support for pointers
- No support for floating-point operations
- Limited standard library support

It's intended for educational purposes and may not handle complex C programs.

## Future Improvements

Potential areas for enhancement include:

- Adding support for more C language features (e.g., structs, pointers)
- Implementing optimizations in the code generation phase
- Adding more robust error handling and reporting
- Extending the standard library support
