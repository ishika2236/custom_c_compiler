OBJECTS = ./build/compiler.o ./build/cprocess.o ./build/helpers/vector.o ./build/helpers/buffer.o
#OBJECTS= list of object files that need to be linked together in order to create final executable

INCLUDES = -I./
#-I option is used to specify directory, included currently  points to look for header files in the current dir


# -g flag is included to generate debugging info 
# all is the default target- it depends on files listed in OBJECTS
all : ${OBJECTS}
	gcc main.c ${INCLUDES} ${OBJECTS} -g -o ./main


./build/compiler.o : ./compiler.c 
	gcc ./compiler.c  ${INCLUDES} -o ./build/compiler.o -g -c

./build/cprocess.o : ./cprocess.c 
	gcc ./cprocess.c ${INCLUDES}  -o ./build/cprocess.o -g -c

./build/helpers/vector.o : ./helpers/vector.c
	gcc ./helpers/vector.c ${INCLUDES}  -o ./build/helpers/vector.o -g -c

./build/helpers/buffer.o : ./helpers/buffer.c
	gcc ./helpers/buffer.c ${INCLUDES}  -o ./build/helpers/buffer.o -g -c

#clean is used to remove generated files to clean up working dir
clean :
	rm ./main
	rm -rf ${OBJECTS}