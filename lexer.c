#include "compiler.h"
#include "helpers/vector.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


#define LEX_GETC_IF(buffer, c, exp)\
    for(c = peekc(); exp; c=peekc())\
    {                               \
        buffer_write(buffer, c);\
        nextc();                \
    }                           \

static struct lex_process* lex_process;
struct token tmptoken;

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

static struct pos lex_file_position()
{
    return lex_process -> pos;
}


void compiler_error(struct compile_process* compiler, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

  
    fprintf(stderr, "on line %d, col %d, in file %s\n", compiler->pos.line, compiler->pos.col, compiler->cfile.abs_path);
    exit(-1);
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

struct token* token_create(struct token* _token){
    memcpy(&tmptoken, _token, sizeof(struct token));
    tmptoken.pos =  lex_file_position();

}
const char* read_number_str()
{
    const char* num = NULL;
    struct buffer* buffer = buffer_create();
    char c = peekc();
    LEX_GETC_IF(buffer, c, (c>='0' && c<='9'));

    buffer_write(buffer, 0x00);
    return buffer_ptr(buffer);
};
unsigned long long read_number()
{
    const char* s = read_number_str();
    return atoll(s);

}

struct token* token_make_number_for_value(unsigned long number)
{
    return token_create(&(struct token){.type = TOKEN_TYPE_NUMBER, .llnum= number});
}
struct token* token_make_number()
{

}
struct token* read_next_token()
{
    struct  token* token = NULL;
    char c = peekc();
    switch (c)
    {
        NUMERIC_CASE:
        token = token_make_number();
        case EOF:
            // We have finished lexical analysis on the file
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
    return LEXICAL_ANALYSIS_ALL_OK;

    return 0;
}