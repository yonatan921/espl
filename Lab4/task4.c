#include <stdio.h>

int digit_count(char * argv){
    int counter = 0;
    for(int i=0; argv[i]; i++){
        char digit = *(argv + i);
        if(digit >= '0' && digit <= '9'){
            counter ++;
        }
    }
    return  counter;
}


int main(int argc, char ** argv) {
    int counter =  digit_count(argv[1]);
    printf("Digit count: %d\n", counter);
    return 0;
}
