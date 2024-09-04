#include "compiler.h"
#include "helpers/vector.h"
#include "helpers/buffer.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


    
static struct lex_process* lex_process;
struct token tmptoken;
struct token* read_next_token();
static char peekc(){
    return lex_process -> functions -> peek_char(lex_process);
}
static char nextc()
{
    char c =  lex_process -> functions ->next_char(lex_process);
    if(lex_process -> current_expression_count > 0)
    {
        buffer_write(lex_process -> parentheses_buffer, c);
    }
    lex_process ->pos.col +=1;
    if(c == '\n'){
        lex_process -> pos.line +=1;
        lex_process -> pos.col = 1;
    }
    return c;
}

static void pushc(char c){
    lex_process->functions -> push_char(lex_process, c);

}

static char assert_next_char(char c)
{
    char next_c = nextc();
    assert(c == next_c);
    return next_c;
}



void compiler_error(struct compile_process* compiler, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

  
    fprintf(stderr, "on line %d, col %d, in file %s\n", compiler->pos.line, compiler->pos.col, compiler->cfile.abs_path);
    exit(COMPILER_FAILED_WITH_ERRORS);
}

void compiler_warning(struct compile_process* compiler, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    
    fprintf(stderr, "on line %d, col %d, in file %s\n", compiler->pos.line, compiler->pos.col, compiler->cfile.abs_path);
    // exit(-1); // Uncomment if you want to terminate on warning
}
struct token* token_create_string(char start_delim, char end_delim)
{
    struct token* token = (struct token* )malloc(sizeof(struct token));
    struct buffer* buffer = buffer_create();
    assert(nextc()==start_delim);
    char c = nextc();
    while (c != end_delim && c != EOF) {
        // printf("%c",c); // debug statement
        buffer_write(buffer, c);
        c = nextc();
    }
    buffer_write(buffer, 0x00);
    token -> sval = buffer_ptr(buffer);
    token -> type = TOKEN_TYPE_STRING;
    token->pos = lex_process->pos;
    // printf("%s\n", token->sval);
    return token;
}

static struct token*  lexer_last_token()
{
    return vector_back_or_null(lex_process -> token_vec);
}

static struct token* handle_whitespace()
{
  
    // char c;
    nextc();
    return read_next_token();
}

const char* read_number_str()
{
    const char* num = NULL;
    struct buffer* buffer = buffer_create();
    char c = peekc();
    while((c = peekc()) >='0' && c<='9' ){ 
        buffer_write(buffer, c); 
        nextc(); 
    }

    buffer_write(buffer, 0x00);
    return buffer_ptr(buffer);
};

unsigned long long read_number()
{
    const char* s = read_number_str();
    return atoll(s);
}
struct token* token_create_number( unsigned long long number)
{
    struct token* token = (struct token*)malloc(sizeof(struct token));
    token -> type =  TOKEN_TYPE_NUMBER;
    token -> llnum =  number;
    token -> pos = lex_process->pos;
    if(lex_process -> current_expression_count>0)
    {
        tmptoken.between_brackets = buffer_ptr(lex_process -> parentheses_buffer);
    }
    // printf("%lld\n", token->llnum);
    return token;
}
static bool op_treated_as_one(char op)
{
    return op == '(' || op == '[' || op == ',' || op == '.' || op == '*' || op == '?';
}
static bool is_single_character(char op)
{
    return op == '+' || 
           op == '-' ||
           op == '=' ||
           op == '*' ||
           op == '&' ||
           op == '|' ||
           op == '(' ||
           op == '%' ||
           op == '~' ||
           op == '!' ||
           op == '<' ||
           op == '>' ||
           op == '.' ||
           op == ',' ||
           op == '?' ;
}
bool op_valid(char* ptr){
    return strcmp(ptr, "++") == 0 ||
           strcmp(ptr, "+=") == 0 ||
           strcmp(ptr, "--") == 0 ||
           strcmp(ptr, "-=") == 0 ||
           strcmp(ptr, "/=") == 0 ||
           strcmp(ptr, "*=") == 0 ||
           strcmp(ptr, "&&") == 0 ||
           strcmp(ptr, "|") == 0 ||
           strcmp(ptr, "||") == 0 ||
           strcmp(ptr, "&") == 0 ||
           strcmp(ptr, "~") == 0 ||
           strcmp(ptr, "!") == 0 ||
           strcmp(ptr, "?") == 0 ||
           strcmp(ptr, "<<") == 0 ||
           strcmp(ptr, ">>") == 0 ||
           strcmp(ptr, "<") == 0 ||
           strcmp(ptr, ">") == 0 ||
           strcmp(ptr, "==") == 0 ||
           strcmp(ptr, "->") == 0 ||
           strcmp(ptr, ",") == 0 ||
           strcmp(ptr, ".") == 0 ||
           strcmp(ptr, "...") == 0 ||
           strcmp(ptr, ">=") == 0 ||
           strcmp(ptr, "<=") == 0 ||
           strcmp(ptr, "+") == 0 ||
           strcmp(ptr, "-") == 0 ||
           strcmp(ptr, "*") == 0 ||
           strcmp(ptr, "/") == 0 ||
           strcmp(ptr, "%") == 0 ||
           strcmp(ptr, "(") == 0 ||
           strcmp(ptr, "[") == 0 ||
           strcmp(ptr, "=") == 0;
           

}
bool is_keyword(const char* keyword)
{
    return strcmp("int",  keyword) == 0 ||
           strcmp("signed", keyword) == 0 ||
           strcmp("char", keyword) == 0 ||
           strcmp("unsigned", keyword) == 0 ||
           strcmp("short", keyword) == 0 ||
           strcmp("float", keyword) == 0 ||
           strcmp("double", keyword) == 0 ||
           strcmp("long long", keyword) == 0 ||
           strcmp("void", keyword) == 0 ||
           strcmp("struct", keyword) == 0 ||
           strcmp("union", keyword) == 0 ||
           strcmp("static", keyword) == 0 ||
           strcmp("return", keyword) == 0 ||
           strcmp("include", keyword) == 0 ||
           strcmp("sizeof", keyword) == 0 ||
           strcmp("if", keyword) == 0 ||
           strcmp("else", keyword) == 0 ||
           strcmp("for", keyword) == 0 ||
           strcmp("while", keyword) == 0 ||
           strcmp("break", keyword) == 0 ||
           strcmp("switch", keyword) == 0 ||
           strcmp("continue", keyword) == 0 ||
           strcmp("case", keyword) == 0 ||
           strcmp("default", keyword) == 0 ||
           strcmp("typedef", keyword) == 0 ||
           strcmp("const", keyword) == 0 ;
           
}
static void lex_new_expression()
{
    lex_process -> current_expression_count ++;
    if(lex_process -> current_expression_count ==1)
    {
        lex_process -> parentheses_buffer = buffer_create();
    }

}
static void flush_back(struct buffer* buffer){
    char* ptr = buffer_ptr(buffer);
    int len = buffer -> len;

    for(int i = len-1; i > 1; i--){
        if(ptr[i]==0x00)
        {
            continue;
        }
        pushc(ptr[i]);
    }
}
char* read_op()
{
    bool single_operator = true;

    char op = nextc();
    // printf("%c\n", op);
    struct buffer* buffer = buffer_create();
    buffer_write(buffer, op);

    if(!op_treated_as_one(op)){
        op =  peekc();

        if(is_single_character(op)){
            buffer_write(buffer, op);
            nextc();
            single_operator = false;
        }
    }
    else{
        buffer_write(buffer, op);
        // nextc();
        if(op =='('){
            lex_new_expression();
        }
    }

    buffer_write(buffer, 0x00);
    char* ptr = (char*) buffer_ptr(buffer);

    if(!single_operator)
    {
        if(!op_valid(ptr))
        {
            flush_back(buffer);
            ptr[1] = 0x00;
        }
    }
    else if(!op_valid(ptr))
    {
        compiler_error(lex_process->compiler, "The operator %s isn't valid ", ptr);
    }
    
    // printf("%s\n", ptr); // debug statement
    return ptr;
}
struct token* token_make_newline()
{
    struct token* token = (struct token*) malloc(sizeof(struct token));
    nextc();
    if(lex_process -> current_expression_count>0)
    {
        tmptoken.between_brackets = buffer_ptr(lex_process -> parentheses_buffer);
    }
    token -> type =  TOKEN_TYPE_NEWLINE;
    token -> pos = lex_process -> pos;
    return token;
}
struct token* token_create_one_line_comment()
{
    struct buffer* buffer = buffer_create();
    char c = peekc();

    while(c != '\n' && c != EOF)
    {
        buffer_write(buffer, c);
        c = nextc();
    }
    if(lex_process -> current_expression_count>0)
    {
        tmptoken.between_brackets = buffer_ptr(lex_process -> parentheses_buffer);
    }
    struct token* token = (struct token*) malloc(sizeof(struct token));
    token->sval = (char*) buffer_ptr(buffer); 
    token->type = TOKEN_TYPE_COMMENT;
    token->pos = lex_process->pos;
    // printf("# %s\n", token->sval); 

    return token;
}

struct token* token_create_multiline_comment()
{
    struct buffer* buffer = buffer_create();
    char c = peekc();
    
    
    while(c != EOF) 
    {
        buffer_write(buffer, c);
        c = nextc();
        
        
        if(c == '*') 
        {
            if(peekc() == '/') 
            {
                nextc(); 
                break;
            }
        }
    }
    
    if(c == EOF) 
    {
        compiler_error(lex_process->compiler, "You did not close this multiline comment, fix it now!\n");
    }
    if(lex_process -> current_expression_count>0)
    {
        tmptoken.between_brackets = buffer_ptr(lex_process -> parentheses_buffer);
    }
    struct token* token = (struct token*) malloc(sizeof(struct token));
    token->sval = (char*) buffer_ptr(buffer);   
    token->type = TOKEN_TYPE_COMMENT;
    token->pos = lex_process->pos;
    // printf("/* %s */\n", token->sval); 

    return token;
}


struct token* token_create_string_or_operator()
{
    
    char op = peekc();
    // printf("%c\n", op);// debug 
    if(op == '<'){
        struct token* last_token = lexer_last_token();
        if(is_token_keyword(last_token, "include"))
        {
            return token_create_string('<','>');
        }
    }
    struct token* token = (struct token*) malloc(sizeof(struct token));
    token -> sval = read_op();
    token -> type = TOKEN_TYPE_OPERATOR;
    token -> pos = lex_process -> pos;
    if(lex_process -> current_expression_count>0)
    {
        tmptoken.between_brackets = buffer_ptr(lex_process -> parentheses_buffer);
    }
    if(op == '(')
    {
        // printf("%c\n",op);
        lex_new_expression();
    }
    
    return token;
}
struct token* handle_comment()
{
    char c = peekc();

    if( c =='/'){
        nextc();
        if(peekc() == '/')
        {
            nextc();
            return token_create_one_line_comment();
        }
        else if (peekc() == '*'){
            nextc();
            return token_create_multiline_comment();
        }
        pushc('/');

        return token_create_string_or_operator();
    }
    return NULL;
}
static void handle_closing_expression()
{
    lex_process -> current_expression_count--;
    // if(lex_process -> current_expression_count < 0){
    //     compiler_error(lex_process -> compiler, "you tried closing an expression that wasn't open in the first place ");
    // }
}

struct token* make_symbol_token()
{
    char c = nextc();

    if(c == ')')
    {
        handle_closing_expression();
    }
    if(lex_process -> current_expression_count>0)
    {
        tmptoken.between_brackets = buffer_ptr(lex_process -> parentheses_buffer);
    }

    struct token* token = (struct token*) malloc(sizeof(struct token));
    token -> cval = c;
    token -> pos = lex_process -> pos;
    token -> type = TOKEN_TYPE_SYMBOL;
    // printf("%c\n",c);

    return token;
}
struct token* make_token_identifer_or_keyword()
{
    struct buffer* buffer = buffer_create();
    char c = nextc();

    for (;( c >= 'a' && c <= 'z') || (c>='A' && c<= 'Z') || (c >='0' && c<='9') || c =='_'; c=nextc())
    {
        buffer_write(buffer,c);
    }

    buffer_write(buffer, 0x00);
    char* ptr = (char*) buffer_ptr(buffer);
    if(lex_process -> current_expression_count>0)
    {
        tmptoken.between_brackets = buffer_ptr(lex_process -> parentheses_buffer);
    }
    struct token* token = (struct token*) malloc(sizeof(struct token));
    token -> sval = ptr;
    token -> pos = lex_process -> pos;
    // printf("%s\n", ptr);

    if( is_keyword(ptr))
    { 
        token -> type = TOKEN_TYPE_KEYWORD;
    }
    else token -> type = TOKEN_TYPE_IDENTIFIER;
    
    return token;

}

struct token* read_special_token()
{
    char c = peekc();
    if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
    {
        return make_token_identifer_or_keyword();
    }
    return NULL;
}

char lex_handle_escape_char(char c)
{
    char ce = 0;
    switch(c)
    {
        case 'n':
            ce = '\n';
            break;
        
        case 't':
            ce = '\t';
            break;

        case '\\':
            ce = '\\';
            break;
        
        case '\'':
            ce = '\'';
            break;
    }
    return ce;
}

struct token* token_make_quote() {
    struct token* token = (struct token*)malloc(sizeof(struct token));
    if (token == NULL) {
        fprintf(stderr, "Memory allocation failed in token_make_quote\n");
        exit(EXIT_FAILURE);
    }

    struct buffer* buffer = buffer_create();
    if (buffer == NULL) {
        fprintf(stderr, "Buffer creation failed in token_make_quote\n");
        exit(EXIT_FAILURE);
    }

    char quote = nextc(); 
    assert(quote == '\''); 

    char c = nextc();
    while (c != quote && c != EOF) {
        buffer_write(buffer, c);
        c = nextc();
    }

    if (c != quote) {
        compiler_error(lex_process->compiler, "Missing closing quote for string literal");
    }

    buffer_write(buffer, 0x00);
    if(lex_process -> current_expression_count>0)
    {
        tmptoken.between_brackets = buffer_ptr(lex_process -> parentheses_buffer);
    }

    token->sval = buffer_ptr(buffer);
    token->type = TOKEN_TYPE_STRING;
    token->pos = lex_process->pos;

    // printf("Char token created: %s\n", token->sval); // Debug code

    return token;
}



struct token* read_next_token()
{
    struct  token* token = NULL;
    // printf("Reading next token at line %d, col %d\n", lex_process->pos.line, lex_process->pos.col); // debug statement
    char c = peekc();
    token = handle_comment();
    if(token)
    {
        return token;
    }
    switch (c)
    {
        case '0' ... '9':
            token = token_create_number(read_number());
            break;

        OPERATOR_EXCLUDING_DIVISION:
            token = token_create_string_or_operator();
            break;

        case '"':
            token = token_create_string('"','"');
            break;

        SYMBOL_CASE:
            token = make_symbol_token();
            break;

        case EOF:
            // We have finished lexical analysis on the file
            
            break;

        case ' ':
        case '\t':
            token = handle_whitespace();
            break;

        case '\'':
            token = token_make_quote();
            break;;

        case '\n':
            token = token_make_newline();
            // printf("-----------------------------------------------------------\n");
            break;
        
        default:
            token = read_special_token();
            if(!token)
            {
                compiler_error(lex_process->compiler, "Unexpected Token");
            }
            
            break;
    }
     lex_process -> compiler -> token_vector_count ++;
   
    return token;
}
int lex(struct lex_process* process)
{
    process -> current_expression_count = 0;
    process -> parentheses_buffer = NULL;
    lex_process =  process;
    process -> pos.filename = process ->compiler->cfile.abs_path;


    struct token* token = read_next_token();
    while(token)
    {
        vector_push(process -> token_vec, token);
        token = read_next_token();
        
    }
    printf("Total count of tokens: %i\n",lex_process->compiler->token_vector_count);
    printf("Total count of tokens: %i\n",vector_count(lex_process -> token_vec));


    
    return LEXICAL_ANALYSIS_ALL_OK;

    return 0;
}
