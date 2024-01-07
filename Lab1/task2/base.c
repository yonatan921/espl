#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int regular_char(char c){
    return c >= 0x20 && c <= 0x7e;
}
char my_get(char c);
/* Ignores c, reads and returns a character from stdin using fgetc. */

char cprt(char c);
/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */

char encrypt(char c);
/* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */

char decrypt(char c);
/* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x20 and 0x7E it is returned unchanged */

char xprt(char c);
/* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */



char* map(char *array, int array_length, char (*f) (char)){
    char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
    for(int i =0; i < array_length; i++){
        mapped_array[i] = f(array[i]);
    }
    /* TODO: Complete during task 2.a */
    return mapped_array;
}

int main(int argc, char **argv){
    /* TODO: Test your code */
    int len = 0;
    char * argument = argv[1];
    while (argument[len] != '\0'){
        len ++;
    }
    char * mapped = map(*argv + 1, len, my_get);
    for(int i =0; i < len; i++){
        printf("%c\n", mapped[i]);
    }
}

char my_get(char c){
    FILE * infile=stdin;
    return getc(infile);
}

char cprt(char c){
    if(regular_char(c)){
        printf("%c\n", c);
    }else{
        printf(".");
    }
    return c;
}
char encrypt(char c){
    if(regular_char(c)){
        c++;
    }
    return c;
}
char decrypt(char c){
    if(regular_char(c)){
        c--;
    }
    return c;
}
char xprt(char c){
    printf("0x%X\n", c);
    return c;
}