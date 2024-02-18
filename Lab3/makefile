all: task1

task1: start.o util.o 
	ld -g -m elf_i386 start.o util.o -o task1

start.o: start.s
	nasm -g -f elf32 start.s -o start.o

util.o: Util.c
	gcc -m32 -Wall -ansi -g -c -nostdlib -fno-stack-protector Util.c -o util.o

clean:
	rm -f *.o task1
