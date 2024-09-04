// compiler process
//contains functions dealing with compiler process operations such as creating actual compiler process itself which is a struct
#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
#include <string.h>
#include "./helpers/vector.h"

struct compile_process* compile_process_create(const char* filename, const char* out_filename, int flags) {
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        // printf("couldn't open input file, check path\n"); // debug statement
        return NULL;
    }
    // printf("input file opened successfully\n"); // debug statement

    FILE *out_file = NULL;
    if (out_filename)
    {
        out_file = fopen(out_filename, "w");
        
        if (!out_file)
        {
            // printf("counldn't open output file, check path\n"); //debug statement
            return NULL;
        }
    }
    
    // printf("output file created successfully\n"); //debug statement

    struct compile_process* process = calloc(1, sizeof(struct compile_process));
    process->flags = flags;
    process->cfile.fp = file;
    process->ofile = out_file;
    process->token_vector_count++;

    return process;
}

char compile_process_next_char(struct lex_process* lex_process)
{
    struct compile_process* compiler = lex_process->compiler;
    compiler->pos.col += 1;
    char c = getc(compiler->cfile.fp);
    if (c == '\n')
    {
        compiler->pos.line +=1 ;
        compiler->pos.col = 1;
    }

    return c;
}

char compile_process_peek_char(struct lex_process* lex_process)
{
    struct compile_process* compiler = lex_process->compiler;
    char c = getc(compiler->cfile.fp);
    ungetc(c, compiler->cfile.fp);
    return c;
}

void compile_process_push_char(struct lex_process* lex_process, char c)
{
    struct compile_process* compiler = lex_process->compiler;
    ungetc(c, compiler->cfile.fp);
}
struct token* parse_process_next_token(struct parse_process* parser)
{
    if(parser -> index < parser -> token_vector_count)
    {
        parser -> index++;
        return (struct token*) vector_at(parser -> token_vector, parser->index);
    }
    return NULL;
}
bool parse_process_match(struct parse_process* parser, int type, const char* value)
{
    struct token* current_token = vector_at(parser -> token_vector, parser->index);
    if(current_token -> type == type && strcmp(current_token -> sval, value) ==0)
    {
        return true;
    }
    return false;
}
bool parse_process_consume(struct parse_process* parser, int type, const char* value) {
    if (parse_process_match(parser, type, value)) {
        parse_process_next_token(parser);
        return true;
    }
    return false;
}