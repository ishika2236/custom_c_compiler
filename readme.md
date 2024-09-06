Here’s the content converted into Markdown:

```markdown
# Main Language Interpreter

This project is a simple interpreter for a custom language called **"Main Language."** The interpreter consists of a **Lexer**, **Parser**, and **Abstract Syntax Tree (AST)** visitor to execute the interpreted code.

## Overview

The Main Language Interpreter allows you to write code in a basic, custom language called "Main Language" and interpret it. The key components of the interpreter are:

- **Lexer**: Breaks down the input source code into tokens.
- **Parser**: Analyzes the tokens and creates an Abstract Syntax Tree (AST).
- **Visitor**: Walks through the AST and executes the code.

### Features

- Interpret commands and expressions from an input file with a `.main` extension.
- Interactive REPL (Read-Eval-Print-Loop) for direct user input.
- Tokenization of input, parsing of expressions, and evaluation of AST nodes.

## Code Flow

### Main Entry (`main.c`):

- Starts by checking if a file is provided as an argument. If the file extension is `.main`, it reads the file and passes its content to the lexer.
- If no file is provided, it runs in interactive mode, continuously prompting the user for input.

### Lexer (`lexer.c`):

- Converts the input source code into tokens, including identifiers, strings, and various symbols (e.g., `=`, `{`, `}`, `;`, etc.).
- Tokenization is performed by the `lexer_get_next_token()` function, which iterates through the input.

### Parser (`parser.c`):

- Consumes tokens produced by the lexer and builds an Abstract Syntax Tree (AST).
- The parser recognizes various constructs like variable definitions, function calls, and compound statements.
- Key parsing functions include `parser_parse_statements()`, `parser_parse_expr()`, and `parser_parse_function_call()`.

### Visitor (`visitor.c`):

- Walks through the AST and evaluates each node (e.g., executing function calls, handling variable definitions, etc.).
- The `visitor_visit()` function is responsible for evaluating nodes based on their type (e.g., `AST_FUNCTION_CALL`, `AST_VARIABLE_DEFINITION`).

## How to Run

### Clone the Repository

```bash
git clone https://github.com/your-username/main-language-interpreter.git
cd main-language-interpreter
```

### Build the Project

Run the following command to build the interpreter:

```bash
make
```

This will compile the source files and generate an executable `main` in the root directory.

### Run the Interpreter

- To run with a script:

```bash
./main tests/sample.main
```

- To run in interactive mode (REPL):

```bash
./main
```

### Clean the Build

To remove the compiled object files and the executable, run:

```bash
make clean
```

## Dependencies

- A C compiler (e.g., GCC)
- Standard C libraries (`stdlib.h`, `stdio.h`, `string.h`)

## Future Enhancements

- Add support for more complex data types.
- Implement advanced language features like loops, conditionals, and error handling.
- Provide a standard library for built-in functions.
```
