//this will contain some core compielr routines
#include "compiler.h"


struct lex_process_functions compiler_lex_functions = {
    .next_char=compile_process_next_char,
    .peek_char=compile_process_peek_char,
    .push_char=compile_process_push_char
};
struct parse_process_functions parse_process_functions =
{
    .next_token = parse_process_next_token,
    .match = parse_process_match,
    .consume = parse_process_consume
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

    process -> token_vec = lex_process -> token_vec;


    // perform parsing
    struct parse_process* parse_process=  create_parse_process(process, &parse_process_functions, NULL);

    if (!parse_process) {
        return COMPILER_FAILED_WITH_ERRORS;
    }
    


    
    //perform code generation..
    return 0;
}