CFLAGS = -m32 -Wall -ansi -fno-stack-protector

all: task2

task2: start.o util.o main.o
	ld -m elf_i386 start.o util.o main.o -o task2

start.o: start.s
	nasm -f elf32 start.s -o start.o

util.o: Util.c
	gcc $(CFLAGS) -c Util.c -o util.o

main.o: main.c
	gcc $(CFLAGS) -c main.c -o main.o

clean:
	rm -f *.o task2
