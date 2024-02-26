#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    // Add additional fields as necessary
} state;

// Function prototypes
void toggleDebugMode(state* s);
void setFileName(state* s);
void setUnitSize(state* s);
void quit(state* s);
void notImplemented(state* s);

void toggleDebugMode(state* s) {
    s->debug_mode = !(s->debug_mode);
    printf("Debug flag now %s\n", s->debug_mode ? "on" : "off");
}

void setFileName(state* s) {
    printf("Enter file name: ");
    scanf("%127s", s->file_name);
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

void quit(state* s) {
    if (s->debug_mode) {
        printf("quitting\n");
    }
    exit(0);
}

void notImplemented(state* s) {
    printf("Not implemented yet\n");
}

void menu(state* s) {
    char* options[] = {
            "Toggle Debug Mode",
            "Set File Name",
            "Set Unit Size",
            "Load Into Memory",
            "Toggle Display Mode",
            "Memory Display",
            "Save Into File",
            "Memory Modify",
            "Quit",
            NULL
    };

    void (functions[])(state) = {
        toggleDebugMode,
                setFileName,
                setUnitSize,
                notImplemented,
                notImplemented,
                notImplemented,
                notImplemented,
                notImplemented,
                quit
    };

    int choice;
    while(1) {
        if (s->debug_mode) {
            printf("Debug: file_name = %s, unit_size = %d, mem_count = %zu\n", s->file_name, s->unit_size, s->mem_count);
        }
        printf("Choose action:\n");
        for (int i = 0; options[i] != NULL; i++) {
            printf("%d-%s\n", i, options[i]);
        }
        scanf("%d", &choice);
        if (choice >= 0 && choice < (sizeof(options) / sizeof(char*)) - 1) {
            (*functions[choice])(s);
        } else {
            printf("Invalid choice\n");
        }
    }
}

int main() {
    state s = {0, "", 1, {0}, 0}; // Initialize state
    menu(&s);
    return 0;
}