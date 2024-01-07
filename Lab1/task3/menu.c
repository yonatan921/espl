#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

int regular_char(char c) {
    return c >= 0x20 && c <= 0x7e;

}
char my_get(char c){
    FILE * infile=stdin;
    return getc(infile);
}

char cprt(char c){
    if(regular_char(c)){
        printf("%c\n", c);
    }else{
        printf(".\n");
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
    printf("%X\n", c);
    return c;
}

struct fun_desc {
    char *name;
    char (*fun)(char);
};
char* map(char *array, int array_length, char (*f) (char)){
    char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
    for(int i =0; i < array_length; i++){
        mapped_array[i] = f(array[i]);
    }
    /* TODO: Complete during task 2.a */
    return mapped_array;
}


int main() {
    char* carray = (char*)(malloc(5*sizeof(char)));
    char buffer[100]; // Buffer to store the input line

    struct fun_desc menu[] = {{ "Get String", my_get },{"Print String", cprt},
            { "Print Hex", xprt }, { "Encrypt", encrypt},
            { "Decrypt", decrypt }, {NULL, NULL} };
    int bounds = sizeof(menu) / (sizeof(struct fun_desc))-1;

    // Read a line from standard input
    printf("Select operation from the following menu: (ctrl^D for exit)\n");
    for (int i = 0; i < bounds; ++i) {
        printf("%d) %s\n", i,  menu[i].name);
    }
    int choose  = fgetc(stdin) - '0';
    if (choose>bounds-1) {
        printf("Not within bounds\n");
        exit(1);
    }
    printf("Option: %d", choose);
    char *mapped = map(carray, 5, menu[choose].fun);
    printf("DONE.\n\n");
    free(carray);
    carray = mapped;


//    fgets(buffer, sizeof(buffer), stdin);
//
//    // Print the input line
//    printf("You entered: %s", buffer);

    return 0;
}