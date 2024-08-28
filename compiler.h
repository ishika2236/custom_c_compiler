#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>

enum
{
    COMPILER_FILE_COMPILED_OK,
    COMPILER_FAILED_WITH_ERRORS
};

struct compile_process{

    //the flags in regards to hoe this file should be compiled
    int flags;

    //sub structure for the input file
    struct compile_process_input_file
    {
        FILE* fp;
        const char* abs_path;
    }cfile;
    
    FILE* ofile; // ==>output file


};

int compile_file(const char* filename, const char* out_filename, int flags);
struct compile_process* compile_process_create(const char* filename, const char* out_filename, int flags);

#endif