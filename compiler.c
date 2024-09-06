//this will contain some core compielr routines
#include "compiler.h"
#include "./helpers/vector.h"


struct lex_process_functions compiler_lex_functions = {
    .next_char=compile_process_next_char,
    .peek_char=compile_process_peek_char,
    .push_char=compile_process_push_char
};
struct parse_process_functions parse_process_functions =
{
    // .next_token = parse_process_next_token,
    .next_token = get_next_token,
    .peek_token= peek_next_token,
    
};
int compile_file(const char* filename, const char* out_filename, int flags)
{
    struct compile_process* process =  compile_process_create(filename, out_filename, flags);
    if(!process)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    //perform lexical analysis
    struct lex_process* lex_process=  lex_process_create(process, &compiler_lex_functions, NULL);

    if (!lex_process) {
        return COMPILER_FAILED_WITH_ERRORS;
    }
    if (lex(lex_process) != LEXICAL_ANALYSIS_ALL_OK) {
        return COMPILER_FAILED_WITH_ERRORS;
    }
    int count = 0;
    process -> token_vec = lex_process -> token_vec;
    printf("size of vector in compile file %d\n", (vector_total(process->token_vec)));
    for(int i = 0 ;i <  lex_process -> token_vec -> total; i++)
    {
        struct token * token =vector_get(lex_process -> token_vec, i);
        printf("token at %d %d : ", token->pos.line, token->pos.col);
        if(token -> type == TOKEN_TYPE_COMMENT|| token -> type == TOKEN_TYPE_IDENTIFIER || token-> type == TOKEN_TYPE_KEYWORD|| token -> type == TOKEN_TYPE_STRING || token->type == TOKEN_TYPE_OPERATOR ){
            printf("%s\n", token->sval);
            count ++;
        }
        else if(token -> type == TOKEN_TYPE_SYMBOL)
        {
            printf("%c\n", token->cval);
            count ++;
        }
        else if(token -> type == TOKEN_TYPE_NEWLINE){
            printf("newline encountered\n");
            count ++;
        }
        else if(token -> type == TOKEN_TYPE_NUMBER){
            printf("%lld\n", token->llnum);
            count ++;
        }
    }
    printf("count of tokens printed: %d\n", count);


    // perform parsing
    struct parse_process* parse_process=  create_parse_process(process, &parse_process_functions, NULL);

    if (!parse_process) {
        return COMPILER_FAILED_WITH_ERRORS;
    }
    // parse_process -> token_vector = process-> token_vec;
    if(parse(parse_process) != PARSER_ANALYSIS_ALL_OK)
    {
        return PARSER_FAILED_WITH_ERRORS;
    }


    
    //perform code generation..
    return 0;
}