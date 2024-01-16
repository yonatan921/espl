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
    char* carray = (char*)(malloc(5 * sizeof(char)));
    char *mapped = NULL;  // Initialize to NULL since it will be allocated dynamically

    struct fun_desc menu[] = {
            {"Get String", &my_get},
            {"Print String", &cprt},
            {"Print Hex", &xprt},
            {"Encrypt", &encrypt},
            {"Decrypt", &decrypt},
            {NULL, NULL}
    };
    // Check how many functions there are dynamically.
    int bounds = sizeof(menu) / sizeof(struct fun_desc) - 1;

    // Read a line from standard input
    while (1) {
        printf("Select operation from the following menu: (ctrl^D for exit)\n");
        for (int i = 0; i < bounds; ++i) {
            printf("%d) %s\n", i, menu[i].name);
        }
        printf("Option : ");
        char input[3];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            // Check for EOF (Ctrl+D) to exit gracefully
            break;
        }
        // We need to remove \n
        input[2] = '\0';
        int choose = atoi(input);

        if (choose >= 0 && choose < bounds) {
            printf("Within bounds\n");
            // Dynamically allocate memory for mapped
            mapped = map(carray, 5, menu[choose].fun);
            printf("DONE.\n\n");
            // Free the old array
            free(carray);
            // Assign carry to mapped
            carray = mapped;

        } else {
            printf("Not Within bounds\n");
            break;
        }
    }
    // No need to free mapped because carry and mapped points to the same place.
    free(carray);
    return 0;
}
