#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define readVirusLen 18
#define virusNameSize 16
#define magicNumberSize 4
#define maxFileName 1000


typedef struct virus {
    unsigned short SigSize; //2
    char virusName[virusNameSize];     //16
    unsigned char* sig;     //8
} virus;

typedef struct link {
    struct link *nextVirus;
    virus *vir;
}link;



void PrintHex(FILE* output, unsigned char* buffer,unsigned short length);
void checkMagicNumber(FILE * input);
int getSize(FILE* file);
void list_print(link *virus_list, FILE* oufFile);
link* list_append(link* virus_list, virus* virus1);
/* Add a new link with the given data to the list (at the end CAN ALSO AT BEGINNING), and return a pointer to the list (i.e., the first link in the list). If the list is null - create a new entry and return a pointer to the entry. */
void list_free(link *virus_list);
/* Free the memory allocated by the list. */
virus* readVirus(FILE* file);
void printVirus(virus* virus, FILE* output);
link* load_signatures(link* link);
link * loadVirusList(FILE * inputFile);
link *print_signatures(link *list);
link * detect_viruse_helper (link *list);
link * neutralize_virus_helper(link *pLink);
link* exit_func(link *list);
void menu_func();
void detect_virus(char *buffer, unsigned int size, link *virus_list);

struct fun_desc {
    char *name;
    link* (*fun)(link*);
};

struct fun_desc menu[] = {
        {"Load signatures",  load_signatures},
        {"Print signatures", print_signatures},
        {"Detect viruses",   detect_viruse_helper},
        {"Fix file",         neutralize_virus_helper},
        {"Quit",             exit_func},
        {NULL, NULL}
};




int main(int argc, char **argv) {

    menu_func();

    return 0;
}

link* exit_func(link *list){
    list_free(list);
    exit(EXIT_SUCCESS);
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
    virus * vir = malloc(sizeof(virus));
    fread(&(vir->SigSize), 2, 1, file);
    fread(vir->virusName, sizeof(char), 16, file);
    vir->sig = malloc(vir->SigSize);
    fread(vir->sig, sizeof(unsigned char), vir->SigSize, file);
    return vir;
}

void printVirus(virus* virus, FILE* output){
    fprintf(output,"virus name: %s\n", (*virus).virusName);
    fprintf(output,"virus size: %d\n", (*virus).SigSize);
    fprintf(output,"signature:\n");
    PrintHex(output, (*virus).sig, (*virus).SigSize);
}

link* load_signatures(link* link){

    char buff[1024];
    char* fileName = NULL;
    printf("Enter signature file name: \n");
    fgets(buff, 1024, stdin);
    sscanf(buff, "%ms", &fileName);
    FILE* file = fopen(fileName, "r");

    if(file==NULL){
        fprintf(stderr,"Reading File Error\n");
        exit(1);
    }
    struct link *head = loadVirusList(file);
    fclose(file);
    free(fileName);
    return head;
}

link *print_signatures(link *list){
    list_print(list, stdout);
    return list;
}

link * loadVirusList(FILE * inputFile){
    int fileSize = getSize(inputFile);
    checkMagicNumber(inputFile);
    int readBytes = magicNumberSize;
    link * head = NULL;

    while(readBytes < fileSize){
        virus* nextVirus = readVirus(inputFile);
        head = list_append(head, nextVirus);
        readBytes = readBytes + readVirusLen + (*nextVirus).SigSize;
    }

    return head;
}

int min(int n, int m){
    return (n > m) ? n : m;
}

void detect_virus(char *buffer, unsigned int size, link *virus_list){
    for(int byteIndex=0; byteIndex < size - 1 ; byteIndex++){
        link * current = virus_list;
        while(current != NULL){
            if(memcmp(current->vir->sig, buffer + byteIndex, min(current->vir->SigSize, size - byteIndex)) == 0 ){
                printf("Starting byte location: %d\n", byteIndex);
                printf("Virus name: %s\n",current->vir->virusName);
                printf("Signature size: %d\n",current->vir->SigSize);
                printf("\n");
            }

            current = current->nextVirus;
        }
    }
}

link * detect_viruse_helper (link *list){
    char buff[maxFileName];
    char* fileName = NULL;
    printf("Enter file's name\n");
    fgets(buff, maxFileName, stdin);
    sscanf(buff, "%ms", &fileName);
    char* infectedBuffer = malloc(10000);   // max 10k
    FILE* file = fopen(fileName, "r");
    fgets(infectedBuffer, maxFileName, file);
    detect_virus(infectedBuffer, 10000, list);
    free(infectedBuffer);
    fclose(file);
    free(fileName);
    return list;
}

void neutralize_virus(char *fileName, int signatureOffset){
    FILE* infected_file = fopen(fileName, "r+");
    fseek(infected_file, signatureOffset, SEEK_SET);
    char retCommand = 0xC3;
    fwrite(&retCommand, 1, 1, infected_file);
    fclose(infected_file);
}

link*neutralize_virus_helper(link* pLink){
    char* tempLocation=malloc(1000);
    printf("Enter the starting byte index:\n");
    fgets(tempLocation, 1000, stdin);
    int initial_location = atoi(tempLocation);
    printf("Enter the infected file name:\n");
    char buffFileName[1024];
    char* infected_file = NULL;
    fgets(buffFileName, 1024, stdin);
    sscanf(buffFileName, "%ms", &infected_file);
    neutralize_virus(infected_file, initial_location);
    free(tempLocation);
    free(infected_file);
    return pLink;
}
void menu_func() {
    int userChoice;
    link* virusList=NULL;
    int menuSize = sizeof(menu) / sizeof(menu[0]) - 1;
    while (1) {
        printf("\nPlease choose a function (0-%d):\n\n", menuSize - 1);//Tb4
        for (int i = 0; menu[i].name != NULL; i++) {//Tb3
            printf("%d) %s function.\n", i, menu[i].name);
        }
        scanf("%d",&userChoice);
        fgetc(stdin);
        if(userChoice==EOF){
            list_free(virusList);
            exit(EXIT_SUCCESS);
        }

        if (userChoice >= 0 && userChoice < menuSize) {
            printf("Within bounds\n");
            virusList = menu[userChoice].fun(virusList);
            printf("\n");
        } else {
            printf("Not within bounds\n");
            list_free(virusList);
            exit(EXIT_SUCCESS);
        }


    }
}