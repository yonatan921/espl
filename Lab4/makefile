all: hexeditplus digit_count

hexeditplus: task1.c
	gcc -m32 -fno-pie -fno-stack-protector -o hexeditplus task1.c

digit_count: task4.c

	gcc -m32  -fno-pie -fno-stack-protector -o digit_count task4.c

clean:
	rm -rf hexeditplus digit_count
