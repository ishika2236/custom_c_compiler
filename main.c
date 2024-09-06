    #include <stdio.h>
    #include "helpers/vector.h"
    #include "compiler.h"


    int main()
    {
        int res = compile_file("./test.c", "./test.s",0);
        if (res == COMPILER_FILE_COMPILED_OK)
        {
            printf("file compiled successfully\n");
        }
        else if(res == COMPILER_FAILED_WITH_ERRORS){
            printf("file compilation failed\n");
        }
        else{
            printf("Unknown error occurred\n");
        }
        return 0;
    }