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
        case '~':                        \
        case '.':                        \
        case ',':                        \
        case '^':                        \
        case '&':                        \
        case '!':                        \
        case '?':                        \
        case '%':                        \
        case '<':                        \
        case '>':                      \
        case '='

    #define SYMBOL_CASE\
        case ')':       \
        case ']':       \
        case ':':       \
        case ';':       \
        case '/':       \
        case '\\':       \
        case '{':       \
        case '}':       \
        case '#':       \
        case  '(':       \
        case  '['       \
        



    struct pos {
        int line;
        int col;
        const char* filename;
    };

    enum {
        LEXICAL_ANALYSIS_ALL_OK,
        LEXICAL_ANALYSIS_INPUT_ERROR
    };
    enum{
        PARSER_ANALYSIS_ALL_OK,
        PARSER_FAILED_WITH_ERRORS
    };

    enum {
        TOKEN_TYPE_KEYWORD,
        TOKEN_TYPE_OPERATOR,
        TOKEN_TYPE_IDENTIFIER,
        TOKEN_TYPE_NUMBER,
        TOKEN_TYPE_SYMBOL,
        TOKEN_TYPE_STRING,
        TOKEN_TYPE_COMMENT,
        TOKEN_TYPE_NEWLINE,
        TOKEN_TYPE_EOF
    };
    enum ast_node_type
    {
        AST_PROGRAM,
        AST_EXPRESSION,
        AST_IDENTIFIER,
        AST_STATEMENT,
        AST_TERM,
        AST_NUMBER,
        AST_ASSIGNMENT,
        AST_BINARY_OP,
        AST_FUNCTION,
        AST_VARIABLE,
        AST_KEYWORD,
        AST_RETURN,
        AST_BLOCK,
        AST_IF,
        AST_ELSE,
        AST_WHILE,
        AST_OPERATOR,
        AST_LITERAL,
        AST_FOR
    };

    struct token {
        int type;
        int flags;
        struct pos pos;
        // char* value;
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

    struct ast_node
    {
        enum ast_node_type type;
        char* value;
        struct ast_node** children; // array of child nodes
        int child_count;
        int child_capacity; // current capacity of children array
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
        struct vector* token_vec;
        int token_vector_count;
        FILE* ofile;
    };

    struct parse_process {
        int flags;
        struct compile_process* compiler;
        struct parse_process_functions* functions;
        struct vector* token_vector;
        int token_vector_count;
        int index;
        void* private;
        
    };
    typedef struct token* (*NEXT_TOKEN)(struct parse_process* parser);
    typedef struct token* (*PEEK_TOKEN)(struct parse_process* parser);

    struct token* get_next_token(struct parse_process* parser);

    struct token* peek_next_token(struct parse_process* parser);



    struct parse_process_functions
    {
        NEXT_TOKEN next_token;  
        PEEK_TOKEN peek_token;

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

    struct ast_node* create_ast_node(enum ast_node_type type);
    struct ast_node* create_ast_node_with_value(enum ast_node_type type, const char* value);
    void add_child(struct ast_node* parent, struct ast_node* child);
    void free_ast_node(struct ast_node* node);
    void print_ast(struct ast_node* node, int indent);
    struct parse_process* create_parse_process(struct compile_process* compiler,struct parse_process_functions* functions, void* private );
    void parser_process_free(struct parse_process* parser);
    void* parser_process_private(struct parse_process* parser);
    struct ast_node* parse_program(struct parse_process* process);
    struct ast_node* parse_declaration(struct parse_process* process);
    struct ast_node* parse_conditional(struct parse_process* process);
    struct ast_node* parse_loop(struct parse_process* process);
    struct ast_node* parse_return(struct parse_process* process);
    struct ast_node* parse_expression(struct parse_process* process);
    struct ast_node* parse_block(struct parse_process* process);
    struct ast_node* parse_statement(struct parse_process* process);
    void syntax_error(const char* message, struct token* token);

    struct token* get_next_token(struct parse_process* parser);
    struct token* peek_next_token(struct parse_process* parser);
    bool parse_process_match(struct parse_process* parser, int type, const char* value);
    bool parse_process_consume(struct parse_process* parser, int type, const char* value);




#endif /* COMPILER_H */