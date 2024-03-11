
all: task0 task1 task2

task0: task0.c
	gcc -m32 -g -o task0 task0.c

task1: task1.c
	gcc -m32 -g -o task1 task1.c

task2: task2.o
	gcc -m32 -c task2.c -o task2.o
	ld -o task2 task2.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

task2.o: task2.c
	gcc -g -Wall -m32 -c -o task2.o task2.c
	nasm -f elf32 start.s -o start.o
	nasm -f elf32 startup.s -o startup.o

clean:
	rm -f *.o task0 task1 task2
