#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define readVirusLen 18
#define virusNameSize 16
#define magicNumberSize 4


typedef struct virus {
    unsigned short SigSize; //2
    char virusName[virusNameSize];     //16
    unsigned char* sig;     //8
} virus;

typedef struct link {
    struct link *nextVirus;
    virus *vir;
}link;

struct fun_desc {
    char *name;
    char (*fun)(char);
};
struct fun_desc menu[] = {
        {"Load signatures",   &my_get},
        {"Print signatures", &cprt},
        {"Detect viruses",    &xprt},
        {"Fix file",      &encrypt},
        {"Quit",      &decrypt},
        {NULL, NULL}
}
void PrintHex(FILE* output, unsigned char* buffer,unsigned short length);
void checkMagicNumber(FILE * input);
int getSize(FILE* file);
void list_print(link *virus_list, FILE* oufFile);
link* list_append(link* virus_list, virus* data);
/* Add a new link with the given data to the list (at the end CAN ALSO AT BEGINNING), and return a pointer to the list (i.e., the first link in the list). If the list is null - create a new entry and return a pointer to the entry. */
void list_free(link *virus_list);
/* Free the memory allocated by the list. */
virus* readVirus(FILE* file);
void printVirus(virus* virus, FILE* output);
link* load_signatures(link* link, const char* unusedFile);
link * loadVirusList(FILE * inputFile);





int main(int argc, char **argv) {
    FILE* input = fopen(argv[1],"r");
    if (input==NULL){
        fprintf(stderr,"Reading File Error\n");
        exit(EXIT_FAILURE);
    }
    FILE* output = stdout;
    int fileSize = getSize(input);

    checkMagicNumber(input);
    int readBytes = magicNumberSize;
    while(readBytes < fileSize){
        virus* nextVirus = readVirus(input);
        printVirus(nextVirus,output);
        readBytes = readBytes + readVirusLen + (*nextVirus).SigSize;
        free(nextVirus);
    }
    return 0;
}

void PrintHex(FILE* output, unsigned char* buffer,unsigned short length) {
    int i =0;
    while (i < length){
        fprintf(output,"%02X ", buffer[i]);
        i++;
    }
    fprintf(output,"\n\n");
}

void checkMagicNumber(FILE * input){
    // Read magic number
    char magic[magicNumberSize];
    fread(magic, sizeof(char), 4, input);

    // Check magic number
    if (strcmp(magic, "VIRL") != 0 && strcmp(magic, "VIRB") != 0) {
        fprintf(stderr, "Error: Invalid magic number\n");
        fclose(input);
        exit(1);
    }

}


int getSize(FILE* file){
    fseek(file, 0L, SEEK_END);
    int fileSize = ftell(file);
    rewind(file);
    return fileSize;
}

/* Print the data of every link in list to the given stream. Each item followed by a newline character. */
void list_print(link *virus_list, FILE* oufFile){
    link* current = virus_list;
    while (current != NULL) {
        printVirus(current->vir, oufFile);
        current = (*current).nextVirus;
    }

    fprintf(oufFile, "\n\n");

}
link* list_append(link* virus_list, virus* virus1){
    link* newLink = (link*)malloc(sizeof(link));
    (*newLink).nextVirus = NULL;
    (*newLink).vir = virus1;

    if (virus_list == NULL) {
        return newLink;
    }

    link* current = virus_list;
    while ((*current).nextVirus != NULL) {
        current = (*current).nextVirus;
    }

    (*current).nextVirus = newLink;

    return virus_list;
}

void list_free(link *virus_list){
    link* current = virus_list;
    while (current != NULL) {
        link* next = (*current).nextVirus;
        free((*current).vir->sig);
        free((*current).vir);
        free(current);
        current = next;
    }
}

virus* readVirus(FILE* file){
    virus* virus  = malloc(sizeof(struct virus));
    if(fread(virus,sizeof(char) ,readVirusLen,file)!=0){
        virus->sig=malloc(virus->SigSize);
        fread(virus->sig,1,virus->SigSize,file);
    }
    return virus;
}
void printVirus(virus* virus, FILE* output){
    fprintf(output,"virus name: %s\n", (*virus).virusName);
    fprintf(output,"virus size: %d\n", (*virus).SigSize);
    fprintf(output,"signature:\n");
    PrintHex(output, (*virus).sig, (*virus).SigSize);
}

link* load_signatures(link* link, const char* unusedFile){
    char* fileName=NULL;
    char buf[BUFSIZ];
    printf("Enter signature file name: ");
    fgets(buf,sizeof(buf),stdin);
    sscanf(buf,"%ms",&fileName);
    FILE* file = fopen(fileName,"rb");
    if(file==NULL){
        fprintf(stderr,"Reading File Error\n");
        exit(EXIT_FAILURE);
    }
    free(fileName);
    struct link *head = load_list(file);
    fclose(file);
    return head;
}

link * loadVirusList(FILE * inputFile){

}
