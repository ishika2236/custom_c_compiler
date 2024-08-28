// compiler process
//contains functions dealing with compiler process operations such as creating actual compiler process itself which is a struct
#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
struct compile_process* compile_process_create(const char* filename, const char* out_filename, int flags)
{
    FILE* file = fopen(filename, "r");

    if(!file)
    {
        return NULL;
    }
    
    FILE* out_file = NULL;
    // this step is if the output filename is passed as params, if not then this step is not valid as sometimes we don't want to build an executable instead we just show it on the terminal
    if(out_filename)
    {
        FILE* out_file = fopen(out_filename, "w");
        if(!out_file)
        {
            return NULL;
        }
    }
    
    struct compile_process* process = calloc(1, sizeof(struct compile_process));
    process -> flags = flags;

};