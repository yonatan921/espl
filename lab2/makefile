all: myshell looper

myshell: myshell.c LineParser.c LineParser.h
	gcc -m32 -o myshell myshell.c LineParser.c

looper: Looper.c
	gcc -m32 -o looper Looper.c

clean:
	rm -rf myshell looper