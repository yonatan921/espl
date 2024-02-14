all: Lab3

Lab3: start.o t0b.o main.o Util.o
	ld -m elf_i386 start.o t0b.o main.o Util.o -o Lab3

start.o: start.s
	nasm -f elf32 start.s -o start.o

t0b.o: t0b.s
	nasm -f elf32 t0b.s -o t0b.o

main.o: main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

Util.o: Util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector Util.c -o Util.o

clean:
	rm -f *.o Lab3
