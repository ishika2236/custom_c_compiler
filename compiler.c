//this will contain some core compielr routines
#include "compiler.h"
int compile_file(const char* filename, const char* out_filename, int flags)
{
    struct compile_process* process =  compile_process_create(filename, out_filename, flags);
    if(!process)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    //perform lexical analysis


    // perform parsing

    
    //perform code generation..
    return 0;
}