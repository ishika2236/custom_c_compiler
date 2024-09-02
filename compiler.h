    #ifndef COMPILER_H
    #define COMPILER_H

    #include <stdio.h>
    #include <stdbool.h>

    #define NUMERIC_CASE\
        case '0':       \
        case '1':       \
        case '2':       \
        case '3':       \
        case '4':       \
        case '5':       \
        case '6':       \
        case '7':       \
        case '8':       \
        case '9'        

    // Position of a file   
    #define OPERATOR_EXCLUDING_DIVISION \
        case '+':                        \
        case '-':                        \
        case '*':                        \
        case '(':                        \
        case '[':                        \
        case '~':                        \
        case '.':                        \
        case ',':                        \
        case '^':                        \
        case '&':                        \
        case '!':                        \
        case '?':                        \
        case '%':                        \
        case '<':                        \
        case '>'                        
    #define SYMBOL_CASE\
        case ')':       \
        case ']':       \
        case ':':       \
        case ';':       \
        case '/':       \
        case '\\':       \
        case '{':       \
        case '}':       \
        case '#'       \


    struct pos {
        int line;
        int col;
        const char* filename;
    };

    enum {
        LEXICAL_ANALYSIS_ALL_OK,
        LEXICAL_ANALYSIS_INPUT_ERROR
    };

    enum {
        TOKEN_TYPE_KEYWORD,
        TOKEN_TYPE_OPERATOR,
        TOKEN_TYPE_IDENTIFIER,
        TOKEN_TYPE_NUMBER,
        TOKEN_TYPE_SYMBOL,
        TOKEN_TYPE_STRING,
        TOKEN_TYPE_COMMENT,
        TOKEN_TYPE_NEWLINE
    };

    struct token {
        int type;
        int flags;
        struct pos pos;

        union {
            char cval;
            const char* sval;
            unsigned int inum;
            unsigned long lnum;
            unsigned long long llnum;
            void* any;
        };

        bool whitespace;
        const char* between_brackets;
    };

    struct lex_process;
    typedef char (*LEX_PROCESS_NEXT_CHAR)(struct lex_process* process);
    typedef char (*LEX_PROCESS_PEEK_CHAR)(struct lex_process* process);
    typedef void (*LEX_PROCESS_PUSH_CHAR)(struct lex_process* process, char c);

    struct lex_process_functions {
        LEX_PROCESS_NEXT_CHAR next_char;
        LEX_PROCESS_PEEK_CHAR peek_char;
        LEX_PROCESS_PUSH_CHAR push_char;
    };

    char compile_process_next_char(struct lex_process* lex_process);
    char compile_process_peek_char(struct lex_process* lex_process);
    void compile_process_push_char(struct lex_process* lex_process, char c);

    struct lex_process {
        struct pos pos;
        struct vector* token_vec;
        struct compile_process* compiler;
        int current_expression_count;
        struct buffer* parentheses_buffer;
        struct lex_process_functions* functions;
        void* private;
    };

    enum {
        COMPILER_FILE_COMPILED_OK,
        COMPILER_FAILED_WITH_ERRORS
    };

    struct compile_process {
        int flags;
        struct pos pos;
        struct compile_process_input_file {
            FILE* fp;
            const char* abs_path;
        } cfile;
        FILE* ofile;
    };

    int compile_file(const char* filename, const char* out_filename, int flags);
    struct compile_process* compile_process_create(const char* filename, const char* out_filename, int flags);
    struct lex_process* lex_process_create(struct compile_process* compiler, struct lex_process_functions* functions, void* private);
    void lex_process_free(struct lex_process* process);
    void* lex_process_private(struct lex_process* process);
    struct vector* lex_process_token(struct lex_process* process);
    int lex(struct lex_process* process);
    void compiler_error(struct compile_process* compiler, const char* msg, ...);
    void compiler_warning(struct compile_process* compiler, const char* msg, ...);
    bool is_token_keyword(struct token* token, char* keyword);


    #endif // COMPILER_H
