#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

// Global variables to manage ELF files
int fd1 = -1, fd2 = -1; // File descriptors
int debug_mode = 0; // Debug mode flag
int num_of_symbols1 = -1, num_of_symbols2 = -1; // Number of symbols
struct stat sb1,sb2; // File status
Elf32_Ehdr *header1 = NULL, *header2 = NULL; // ELF headers
void *map_start1 = NULL, *map_start2 = NULL; // Mapped memory regions

#define ELF_FILE_COUNT 2 // Number of ELF files you can handle


// Function prototypes
void examine_elf_file();
void print_examine_inf(int num);
void toggle_debug_mode();
void print_section_names();
void print_symbols();
void check_files_for_merge();
void merge_elf_files();
void notImplemented();
void quit_program();

void toggle_debug_mode() {
    debug_mode = !(debug_mode);
    printf("Debug flag now %s\n", debug_mode ? "ON" : "OFF");
}

void examine_elf_file() {
    printf("Please enter the ELF filename: ");
    char buffer[100];
    char filename[100];
    fgets(buffer, 100, stdin);
    sscanf(buffer, "%s", filename);

    if (fd1 != -1 && fd2 != -1) {
        printf("Error: You can only handle %d ELF files\n", ELF_FILE_COUNT);
        return;
    }
    if (fd1 == -1) {
        fd1 = open(filename, O_RDWR);
        if (fd1 == -1) {
            perror("Error: open");
            return;
        }
        if (fstat(fd1, &sb1) == -1) {
            perror("Error: file size");
            close(fd1);
            fd1 = -1;
            return;
        }
        if ((map_start1 = mmap(NULL, sb1.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd1, 0)) == MAP_FAILED) {
            perror("Error: mapping file into memory failed");
            close(fd1);
            fd1 = -1;
            return;
        }
        header1 = (Elf32_Ehdr *) map_start1;
        print_examine_inf(1);
        //save num symbol
        Elf32_Shdr *shdr = (Elf32_Shdr *) (map_start1 + header1->e_shoff);
        int j=0;
        num_of_symbols1 = 0;
        while (j<header1->e_shnum){
            if (shdr[j].sh_type == SHT_SYMTAB || shdr[j].sh_type == SHT_DYNSYM){
                num_of_symbols1 = shdr[j].sh_size / shdr[j].sh_entsize;
            }
            j++;
        }
    }else{
        fd2 = open(filename, O_RDWR);
        if (fd2 == -1) {
            perror("Error: could not open file ");
            return;
        }
        if (fstat(fd2, &sb2) == -1) {
            perror("Error: file size");
            close(fd2);
            fd2 = -1;
            return;
        }
        if ((map_start2 = mmap(NULL, sb2.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd2, 0)) == MAP_FAILED) {
            perror("Error: mapping file into memory failed");
            close(fd2);
            fd2 = -1;
            return;
        }
        header2 = (Elf32_Ehdr *) map_start1;
        print_examine_inf(2);
        //save num symbol
        Elf32_Shdr *shdr = (Elf32_Shdr *) (map_start2 + header2->e_shoff);
        int j=0;
        num_of_symbols2 = 0;
        while (j<header2->e_shnum){
            if (shdr[j].sh_type == SHT_SYMTAB || shdr[j].sh_type == SHT_DYNSYM){
                num_of_symbols2 = shdr[j].sh_size / shdr[j].sh_entsize;
            }
            j++;
        }


    }
}

void print_examine_inf(int num){
    if(num==1){
        // Once you verified your output, extend examine to print the following information from the header:
        //
        //    Bytes 1,2,3 of the magic number (in ASCII). Henceforth, you should check that the number is consistent with an ELF file, and refuse to continue if it is not.
        //    The data encoding scheme of the object file.
        //    Entry point (hexadecimal address).
        //    The file offset in which the section header table resides.
        //    The number of section header entries.
        //    The size of each section header entry.
        //    The file offset in which the program header table resides.
        //    The number of program header entries.
        //    The size of each program header entry.
        printf("Magic number: %c%c%c\n", header1->e_ident[EI_MAG1], header1->e_ident[EI_MAG2], header1->e_ident[EI_MAG3]);
        printf("The data encoding scheme of the object file: %s\n", (header1->e_ident[EI_DATA] == ELFDATA2LSB) ? "little endian" : (header1->e_ident[EI_DATA] == ELFDATA2MSB) ? "big endian" : "unknown");
        printf("Entry point (hexadecimal address): 0x%x\\n", header1->e_entry);
        printf("The file offset in which the section header table resides: %lu\n", (unsigned long)header1->e_shoff);
        printf("The number of section header entries: %u\n", header1->e_shnum);
        printf("The size of each section header entry: %u\n", header1->e_shentsize);
        printf("The file offset in which the program header table resides: %u\n", header1->e_phoff);
        printf("The number of program header entries: %u\n", header1->e_phnum);
        printf("The size of each program header entry: %u\n", header1->e_phentsize);
    }else {
        printf("Magic number: %c%c%c\n", header2->e_ident[EI_MAG1], header2->e_ident[EI_MAG2],
               header2->e_ident[EI_MAG3]);
        printf("The data encoding scheme of the object file: %s\n",
               (header2->e_ident[EI_DATA] == ELFDATA2LSB) ? "little endian" : (header2->e_ident[EI_DATA] == ELFDATA2MSB)
                                                                              ? "big endian" : "unknown");
        printf("Entry point (hexadecimal address): 0x%x\\n", header2->e_entry);
        printf("The file offset in which the section header table resides: %lu\n", (unsigned long) header2->e_shoff);
        printf("The number of section header entries: %u\n", header2->e_shnum);
        printf("The size of each section header entry: %u\n", header2->e_shentsize);
        printf("The file offset in which the program header table resides: %u\n", header2->e_phoff);
        printf("The number of program header entries: %u\n", header2->e_phnum);
        printf("The size of each program header entry: %u\n", header2->e_phentsize);
    }
}

void print_section_names() {
    notImplemented();
}
void print_symbols() {
    notImplemented();
}
void check_files_for_merge() {
    notImplemented();
}
void merge_elf_files() {
    notImplemented();
}
void quit_program() {
    if (fd1 != -1) {
        close(fd1);
    }
    if (fd2 != -1) {
        close(fd2);
    }
    if (map_start1 != NULL) {
        munmap(map_start1, sb1.st_size);
    }
    if (map_start2 != NULL) {
        munmap(map_start2, sb2.st_size);
    }
    exit(EXIT_SUCCESS);
}
void notImplemented(){
    printf("Not implemented yet\n");
}


struct fun_desc {
    char *name;
    void (*func)();
};

//Choose action:
//0-Toggle Debug Mode
//1-Examine ELF File
//2-Print Section Names
//3-Print Symbols
//4-Check Files for Merge
//5-Merge ELF Files
//6-Quit
struct fun_desc menu[] = {
        {"Toggle Debug Mode", toggle_debug_mode},
        {"Examine ELF File", examine_elf_file},
        {"Print Section Names", print_section_names},
        {"Print Symbols", print_symbols},
        {"Check Files for Merge", check_files_for_merge},
        {"Merge ELF Files", merge_elf_files},
        {"Quit", quit_program},
        {NULL, NULL}
};

void menu_func() {
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
            menu[userChoice].func();
            printf("\n");
        } else {
            exit(EXIT_SUCCESS);
        }


    }
}
int main() {
    menu_func();
    return 0;
}

