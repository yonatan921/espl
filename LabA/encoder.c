#include <stdio.h>
#include <string.h>

int isDigit(int c){
    return c <= '9' && c >= '0';
}

int isUpperCase(int c){
    return c <= 'Z' && c >= 'A';
}

int main(int argc, char **argv) {
    int numOfLetters = 'Z' - 'A' + 1;
    int numOfDigits = '9' - '0' + 1;
    int debug = 1;
    int posNeg = 1;
    char *encoderString = NULL;
    int indexInEncoder = 2;
    FILE * outfile=stdout;
    FILE * infile=stdin;
    for (int i = 1; i < argc; ++i) {
        if(strcmp("+D", argv[i]) == 0){
            debug = 1;
        } else if(strcmp("-D", argv[i]) == 0){
            debug = 0;
        }if(debug){
            fprintf(stderr, "%s\n", argv[i]);
        }if(strncmp(argv[i], "+E", 2) == 0){
            posNeg = 1;
            encoderString = argv[i];
        } else if(strncmp(argv[i], "-E", 2) == 0){
            posNeg = -1;
            encoderString = argv[i];
        } if(strncmp(argv[i],"-O", 2) == 0){
            outfile = fopen(argv[i] + 2, "w");
            if(outfile == NULL){
                fprintf(stderr, "Cannot set file %s as outfile\n", argv[i] + 2);
                return 1;
            }
        } if(strncmp(argv[i],"-I", 2) == 0) {
            infile = fopen(argv[i] + 2, "r");
            if(infile == NULL){
                fprintf(stderr, "Cannot set file %s as infile\n", argv[i] + 2);
                return 1;
            }
        }
    }
    if(encoderString == NULL){
        fprintf(stderr, "Encoder wasn't found");
        return 1;
    }

    while (!feof(infile)){
        if(encoderString[indexInEncoder] == '\0'){
            indexInEncoder = 2;
        }
        int offset = (encoderString[indexInEncoder] - '0') * posNeg;
        char c = fgetc(infile);
        if(isDigit(c)) {
            c = '0' + ((c - '0' + offset) % numOfDigits);
            if (c < '0') c = c + numOfDigits;
        }else if(isUpperCase(c)){
            c = 'A' + ((c - 'A' +offset) % numOfLetters);
            if (c < 'A') c = c + numOfLetters;
        }
        indexInEncoder++;
        fputc(c, outfile);
    }
    return 0;
}
