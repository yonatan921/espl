#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char debug_mode;
    char display_mode; // 0 for decimal, 1 for hex
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
} state;

// Function prototypes
void toggleDebugMode(state* s);
void setFileName(state* s);
void setUnitSize(state* s);
void quit(state* s);
void notImplemented(state* s);
static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};
void toggleDebugMode(state* s) {
    s->debug_mode = !(s->debug_mode);
    printf("Debug flag now %s\n", s->debug_mode ? "on" : "off");
}

void setFileName(state* s) {
    printf("Enter file name: ");
    scanf("%100s", s->file_name);
    if (s->debug_mode) {
        fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
    }
}

void setUnitSize(state* s) {
    int size;
    printf("Enter unit size (1, 2, or 4): ");
    scanf("%d", &size);
    if (size == 1 || size == 2 || size == 4) {
        s->unit_size = size;
        if (s->debug_mode) {
            fprintf(stderr, "Debug: set size to %d\n", s->unit_size);
        }
    } else {
        printf("Invalid unit size\n");
    }
}

void loadIntoMemory(state* s) {
    if (strcmp(s->file_name, "") == 0) {
        printf("Error: file name is empty.\n");
        return;
    }

    FILE *file = fopen(s->file_name, "rb");
    if (!file) {
        printf("Error: unable to open file %s.\n", s->file_name);
        return;
    }

    printf("Please enter <location> <length>: ");
    char input[100];
    unsigned int location, length;
    fgets(input, sizeof(input), stdin); // Read the entire line
    sscanf(input, "%x %u", &location, &length); // Parse location as hex and length as decimal

    if (s->debug_mode) {
        printf("Debug: file_name = %s, location = %X, length = %u\n", s->file_name, location, length);
    }

    fseek(file, location, SEEK_SET);
    s->mem_count = fread(s->mem_buf, s->unit_size, length, file);
    if (s->mem_count < length) {
        printf("Warning: Only %zu units were loaded due to EOF or read error.\n", s->mem_count);
    } else {
        printf("Loaded %zu units into memory.\n", s->mem_count);
    }

    fclose(file);
}

void toggleDisplayMode(state* s) {
    s->display_mode = !(s->display_mode);
    if (s->display_mode) {
        printf("Display flag now on, hexadecimal representation\n");
    } else {
        printf("Display flag now off, decimal representation\n");
    }
}

void memoryDisplay(state* s) {
    unsigned int address, length;
    printf("Enter address and length: ");
    scanf("%x %u", &address, &length); // Address in hex, length in decimal
    unsigned char* start ;
    if(address == 0){
        start = s->mem_buf;
    } else{
        start = (unsigned char*)address;
    }

    printf("%s\n", s->display_mode ? "Hexadecimal" : "Decimal");
    for (unsigned int i = 0; i < length; ++i) {
        unsigned int val = 0;
        memcpy(&val, start + i * s->unit_size, s->unit_size); // Copy unit_size bytes to val
        if (s->display_mode) {
            // Hexadecimal
            printf(hex_formats[s->unit_size / 2 - 1], val);
        } else {
            // Decimal
            printf(dec_formats[s->unit_size / 2 - 1], val);
        }
    }
}

void quit(state* s) {
    if (s->debug_mode) {
        printf("quitting\n");
    }
    exit(0);
}

void notImplemented(state* s) {
    printf("Not implemented yet\n");
}


struct fun_desc {
    char *name;
    void (*fun)(state*);
};


struct fun_desc menu[] = {
        {"Toggle Debug Mode", toggleDebugMode},
        {"Set File Name", setFileName},
        {"Set Unit Size", setUnitSize},
        {"Load Into Memory", loadIntoMemory},
        {"Toggle Display Mode", toggleDisplayMode},
        {"Memory Display", memoryDisplay},
        {"Save Into File", notImplemented},
        {"Memory Modify", notImplemented},
        {"Quit", quit},
        {NULL, NULL}
};


void menu_func(state* s) {
    int userChoice;
    int menuSize = sizeof(menu) / sizeof(menu[0]) - 1;
    while (1) {
        printf("\nPlease choose a function (0-%d):\n\n", menuSize - 1);//Tb4
        for (int i = 0; menu[i].name != NULL; i++) {
            printf("%d) %s function.\n", i, menu[i].name);
        }
        scanf("%d",&userChoice);
        fgetc(stdin);
        if(userChoice==EOF){
            exit(EXIT_SUCCESS);
        }

        if (userChoice >= 0 && userChoice < menuSize) {
            menu[userChoice].fun(s);
            printf("\n");
        } else {
            exit(EXIT_SUCCESS);
        }


    }
}

int main() {
    state s = {0, 0, "", 1, {0}, 0};
    menu_func(&s);
    return 0;
}