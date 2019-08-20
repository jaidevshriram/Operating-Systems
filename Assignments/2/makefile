#Compiler
CC=gcc

#Compile Time Flags
CFLAGS=-I

#Include other directories to search for header files
INCLUDES = -I/cd

#Source Files
SRCS = main.c ./cd/changedir.cd

#Define C Object Files
OBJS = $(SRCS:.c=.o)

#Name the Executable
MAIN = shell

all: $(MAIN)
	@echo Simple shell has been compiled

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


shell: cd/changedir.h main.c
	gcc -o shell cd/changedir.h main.c