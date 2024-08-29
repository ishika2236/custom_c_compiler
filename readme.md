
# C Compiler Project

This project is a simple compiler written in C, designed to compile C source files into executable code. The compiler consists of several key stages, including lexical analysis, parsing, and code generation. 

## Project Structure

- **compiler.c**: Handles the main compilation process, including calling the various stages of compilation.
- **cprocess.c**: Manages the creation of the compile process, including file handling and setting up the compilation environment.
- **main.c**: Contains the entry point of the application, which initiates the compilation process.
- **helpers/vector.c**: Utility functions for managing dynamic arrays (vectors).
- **helpers/buffer.c**: Utility functions for managing buffers.

## Compilation Steps

1. **Project Preparation**:
   - Compile necessary object files using the Makefile.
   - Object files are linked together to create the final executable.

2. **Lexical Analysis**:
   - The first step in the compilation process, where the source code is broken down into tokens.
   - A token has a type and a value
   - The part of a software program that performs lexical analysis is called lexer
   - Benefits:
        - idenitfy source code keywords, symbols, operators and more
        - less work for the parser
        - token info is in correct format (i.e. numbers are actually integers not string once processed by the lexers)
        - unecessary info such as blank spaces are discarded

    - Token types in our compiler:
        - TOKEN_TYPE_IDENTIFIER (words that are not keywords)
        - TOKEN_TYPE_KEYWORD (unsigned, signed, int,, float, double, long, void, struct etc)
        - TOKEN_TYPE_OPERATOR (+,-,/,* etc)
        - TOKEN_TYPE_SYMBOL ('',"",;,:,etc)
        - TOKEN_TYPE_NUMBER (1,2,3,23 etc)
        - TOKEN_TYPE_STRING  ("ehllo world", etc)
        - TOKEN_TYPE_COMMENT (c program comment)
        - TOKEN_TYPE_NEWLINE (new line in program)

3. **Parsing**:
   - After lexical analysis, the compiler parses the tokens to create a syntax tree, which represents the hierarchical structure of the source code.

4. **Code Generation**:
   - Finally, the compiler generates the target code, which can be executed. This may include generating assembly code, machine code, or another high-level language.

## Makefile

The provided Makefile automates the build process. Key targets include:

- **all**: Compiles all necessary object files and links them to create the final executable.
- **clean**: Removes generated files, helping to clean up the working directory.

### Example Makefile Targets

```makefile
all : ${OBJECTS}
	gcc main.c ${INCLUDES} ${OBJECTS} -g -o ./main

./build/compiler.o : ./compiler.c 
	gcc ./compiler.c  ${INCLUDES} -o ./build/compiler.o -g -c

./build/cprocess.o : ./cprocess.c 
	gcc ./cprocess.c ${INCLUDES}  -o ./build/cprocess.o -g -c

./build/helpers/vector.o : ./helpers/vector.c
	gcc ./helpers/vector.c ${INCLUDES}  -o ./build/helpers/vector.o -g -c

./build/helpers/buffer.o : ./helpers/buffer.c
	gcc ./helpers/buffer.c ${INCLUDES}  -o ./build/helpers/buffer.o -g -c

clean :
	rm ./main
	rm -rf ${OBJECTS}
```

## Usage

To compile and run this project, use the following commands:

1. **Compile the project**:
   ```bash
   make all
   ```

2. **Run the compiler**:
   ```bash
   ./main
   ```

3. **Clean the project**:
   ```bash
   make clean
   ```

