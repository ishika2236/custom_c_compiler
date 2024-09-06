#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
#include <string.h>
#include "./helpers/vector.h"

struct compile_process* compile_process_create(const char* filename, const char* out_filename, int flags) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        // printf("couldn't open input file, check path\n"); // debug statement
        return NULL;
    }
    // printf("input file opened successfully\n"); // debug statement

    FILE *out_file = NULL;
    if (out_filename) {
        out_file = fopen(out_filename, "w");
        if (!out_file) {
            // printf("couldn't open output file, check path\n"); //debug statement
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

char compile_process_next_char(struct lex_process* lex_process) {
    struct compile_process* compiler = lex_process->compiler;
    compiler->pos.col += 1;
    char c = getc(compiler->cfile.fp);
    if (c == '\n') {
        compiler->pos.line += 1;
        compiler->pos.col = 1;
    }

    return c;
}

char compile_process_peek_char(struct lex_process* lex_process) {
    struct compile_process* compiler = lex_process->compiler;
    char c = getc(compiler->cfile.fp);
    ungetc(c, compiler->cfile.fp);
    return c;
}

void compile_process_push_char(struct lex_process* lex_process, char c) {
    struct compile_process* compiler = lex_process->compiler;
    ungetc(c, compiler->cfile.fp);  
}

struct token* get_curr_token(struct parse_process* parser)
{
   
     if (parser->index < parser->token_vector_count) {
        return vector_get(parser->token_vector, parser->index-1);
    }
    return NULL;
}

// bool parse_process_match(struct parse_process* parser, int type, const char* value) {
//     struct token* current_token = vector_get(parser->token_vector, parser->index);
//     if (current_token->type == type && strcmp(current_token->sval, value) == 0) {
//         return true;
//     }
//     return false;
// }

// bool parse_process_consume(struct parse_process* parser, int type, const char* value) {
//     if (parse_process_match(parser, type, value)) {
//         get_next_token(parser);
//         return true;
//     }
//     return false;
// }

struct token* get_next_token(struct parse_process* parser) {
    if (parser->index < parser->token_vector_count) {
        return vector_get(parser->token_vector, parser->index++);
    }
    return NULL;
}

struct token* peek_next_token(struct parse_process* parser) {
    if (parser->index < parser->token_vector_count) {
        return vector_get(parser->token_vector, parser->index);
    }
    return NULL;
}
// bool parse_process_match(struct parse_process* parser, int type, const char* value) {
//     struct token* token = peek_next_token(parser);
//     if (token && token->type == type && 
//         (value == NULL || strcmp(token->sval, value) == 0)) {
//         parser->index++;
//         return true;
//     }
//     return false;
// }
// void parse_process_expect(struct parse_process* parser, int type, const char* value) {
//     if (!parse_process_match(parser, type, value)) {
//         struct token* token = peek_next_token(parser);
//         compiler_error(parser->compiler, "Expected token type %d with value '%s', but got type %d with value '%s'",
//                        type, value, token ? token->type : -1, token ? token->sval : "EOF");
//     }
// }
