#all: myshell mypipeline
#
#myshell: myshell.c mypipeline.c
#	gcc -m32 -o myshell myshell.c mypipeline.c
#
#mypipeline: mypipeline.c
#	gcc -m32 -o mypipeline mypipeline.c
#
#
#clean:
#	rm -rf myshell mypipeline
# Define compiler and flags
# Define compiler and flags
CC=gcc
CFLAGS=-m32

all: myshell mypipeline

# Compile myshell program with LineParser.c
myshell: myshell.c LineParser.c
	$(CC) $(CFLAGS) -o myshell myshell.c LineParser.c

# Compile mypipeline program
mypipeline: mypipeline.c
	$(CC) $(CFLAGS) -o mypipeline mypipeline.c

clean:
	rm -rf myshell mypipeline
