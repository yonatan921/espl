all: myshell looper mypipe

myshell: myshell.c LineParser.c LineParser.h
	gcc -m32 -o myshell myshell.c LineParser.c

looper: Looper.c
	gcc -m32 -o looper Looper.c

mypipe: mypipe.c
	gcc -m32 -o mypipe mypipe.c

clean:
	rm -rf myshell looper mypipe
