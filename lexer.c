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
    printf("%s\n", token->sval);
    return token;
}
struct token* token_create_number( unsigned long long number)
{
    struct token* token = (struct token*)malloc(sizeof(struct token));
    token -> type =  TOKEN_TYPE_NUMBER;
    token -> llnum =  number;
    token -> pos = lex_process->pos;
    printf("%lld\n", token->llnum);
    return token;
}
static struct token*  lexer_last_token()
{
    return vector_back_or_null(lex_process -> token_vec);
}

static struct token* handle_whitespace()
{
  
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



struct token* read_next_token()
{
    struct  token* token = NULL;
    char c = peekc();
    
    switch (c)
    {
         case '0' ... '9':
            printf("Reading next token at line %d, col %d\n", lex_process->pos.line, lex_process->pos.col);//debug statement
            token = token_create_number(read_number());
        break;
        case '"':
            printf("Reading next token at line %d, col %d\n", lex_process->pos.line, lex_process->pos.col);//debug statement
            token = token_create_string('"','"');
            break;
        case EOF:
            // We have finished lexical analysis on the file
            break;

        case ' ':
        case '\t':
            token = handle_whitespace();
        break;
        
        default:
            compiler_error(lex_process->compiler, "Unexpected Token");
            break;
    }
   
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
    printf("%i\n",vector_count(process->token_vec));
    
    return LEXICAL_ANALYSIS_ALL_OK;

    return 0;
}
