#include <stdio.h>
#include <stdlib.h>
#include "./helpers/vector.h"
#include "compiler.h"


struct parse_process* create_parse_process(struct compile_process* compiler,struct parse_process_functions* functions, void* private )
{
    struct parse_process* parser = (struct parse_process*) malloc(sizeof(struct parse_process));
    parser -> functions = functions;
    parser -> private = private;
    parser -> compiler = compiler;
    parser -> token_vector = compiler -> token_vec;
    parser -> index = 0;
    parser -> token_vector_count = compiler -> token_vector_count;
    return parser;
}

void parser_process_free(struct parse_process* parser)
{
    free(parser -> token_vector);
    free(parser);
}
void* parser_process_private(struct parse_process* parser)
{
    return parser -> private;

}


