all: mypipeline

myshell: myshell.c mypipeline.c
	gcc -m32 -o myshell myshell.c mypipeline.c

mypipeline: mypipeline.c
	gcc -m32 -o mypipeline mypipeline.c


clean:
	rm -rf myshell mypipeline
