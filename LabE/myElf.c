#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>

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
char *getSectionHeaderType(int type) {
    switch (type) {
        case SHT_NULL:
            return "NULL";
        case SHT_PROGBITS:
            return "PROGBITS";
        case SHT_SYMTAB:
            return "SYMTAB";
        case SHT_STRTAB:
            return "STRTAB";
        case SHT_RELA:
            return "RELA";
        case SHT_HASH:
            return "HASH";
        case SHT_DYNAMIC:
            return "DYNAMIC";
        case SHT_NOTE:
            return "NOTE";
        case SHT_NOBITS:
            return "NOBITS";
        case SHT_REL:
            return "REL";
        case SHT_SHLIB:
            return "SHLIB";
        case SHT_DYNSYM:
            return "DYNSYM";
        case SHT_INIT_ARRAY:
            return "INIT_ARRAY";
        case SHT_FINI_ARRAY:
            return "FINI_ARRAY";
        case SHT_PREINIT_ARRAY:
            return "PREINIT_ARRAY";
        case SHT_GROUP:
            return "GROUP";
        case SHT_SYMTAB_SHNDX:
            return "SYMTAB_SHNDX";
        default:
            return "UNKNOWN";
    }
}


void print_section_names() {
    // Check if there's at least one file mapped
    if (fd1 == -1 && fd2 == -1) {
        printf("Error: No ELF file is currently open.\n");
        return;
    }
    for (int filenum = 1; filenum <= 2; filenum++) {
        // Determine which file to work with
        Elf32_Ehdr *header = (filenum == 1) ? header1 : header2;
        void *map_start = (filenum == 1) ? map_start1 : map_start2;
        struct stat sb = (filenum == 1) ? sb1 : sb2;
        if (header == NULL) continue; // Skip if no file is loaded

        printf("File %d sections:\n", filenum);

        // Get section header string table index
        int shstrndx = header->e_shstrndx;
        Elf32_Shdr *shdr = (Elf32_Shdr *)(map_start + header->e_shoff);
        char *strTab = (char *)(map_start + shdr[shstrndx].sh_offset);
        printf("[index]\tsection_name\tsection_address\tsection_offset\tsection_size\tsection_type\n");
        int section_header = 0;
        while (section_header < header->e_shnum) {
            printf("[%d]\t%s\t0x%x\t0x%x\t%u\t%s\n",
                   section_header,
                   &strTab[shdr[section_header].sh_name],
                   shdr[section_header].sh_addr,
                   shdr[section_header].sh_offset,
                   shdr[section_header].sh_size,
                   getSectionHeaderType(shdr[section_header].sh_type));
            section_header++;
        }
        // Print debug information if debug mode is on
        if (debug_mode) {
            printf("Debug: shstrndx = %d\n", shstrndx);
            printf("Debug: Section name string table offset = 0x%lx\n", (unsigned long)shdr[shstrndx].sh_offset);
            printf("shstrndx name: %s\n", &strTab[shdr[shstrndx].sh_name]);
            for (int i = 0; i < header->e_shnum; i++) {
                printf("Debug: Section [%d] name offset = 0x%lx\n", i, (unsigned long)shdr[i].sh_name);
            }
        }

    }
}

void print_symbols() {
    // Iterate through both files if they are open
    for (int filenum = 1; filenum <= 2; filenum++) {
        Elf32_Ehdr *header = (filenum == 1) ? header1 : header2;
        void *map_start = (filenum == 1) ? map_start1 : map_start2;
        if (header == NULL) {
            printf("File %d is not open.\n", filenum);
            continue; // Skip if the file is not loaded
        }

        // Locate .strtab section for symbol names
        Elf32_Shdr *shdr = (Elf32_Shdr *)(map_start + header->e_shoff);
        Elf32_Shdr *strtab_shdr = NULL;//and strtab_shdr is a pointer to the section header of the string table section ,which contains the names of the symbols.
        Elf32_Shdr *symtab_shdr = NULL;//symtab_shdr is a pointer to the section header of the symbol table section, which contains the symbols themselves.
        char *strtab = NULL;

        // Find .strtab and .symtab sections
        for (int i = 0; i < header->e_shnum; i++) {//Iterate through the section headers to find the string table and symbol table sections.
            if (shdr[i].sh_type == SHT_STRTAB && i != header->e_shstrndx) {
                strtab_shdr = &shdr[i];
                strtab = (char *)(map_start + shdr[i].sh_offset);
            } else if (shdr[i].sh_type == SHT_SYMTAB) {
                symtab_shdr = &shdr[i];
            }
        }

        if (!symtab_shdr || !strtab_shdr) {//If the symbol table or the string table is not found, print an error message and continue to the next file.
            printf("Symbol table or string table not found.\n");
            continue;
        }

        Elf32_Sym *symtab = (Elf32_Sym *)(map_start + symtab_shdr->sh_offset);
        int num_symbols = symtab_shdr->sh_size / symtab_shdr->sh_entsize;
        printf("File %d symbols:\n", filenum);
        printf("[index]\tvalue\tsection_index\tsection_name\tsymbol_name\n");
        int symbolIndex = 0; // representing the index of the current symbol in the symbol table
        while (symbolIndex < num_symbols) {
            Elf32_Sym currentSymbol = symtab[symbolIndex]; // Access the current symbol
            int sectionIndex = currentSymbol.st_shndx; // Index of the section in which this symbol is defined
            // Determine the section name. If the section index is valid, find the section's name; otherwise, label it as "N/A"
            char *sectionName = (sectionIndex < header->e_shnum) ?
                                (char *)(map_start + shdr[header->e_shstrndx].sh_offset + shdr[sectionIndex].sh_name) :
                                "N/A";
            char *symbolName = &strtab[currentSymbol.st_name]; // Name of the current symbol

            // Print the symbol information
            printf("[%d]\t0x%x\t%d\t%s\t%s\n",
                   symbolIndex,
                   currentSymbol.st_value,
                   sectionIndex,
                   sectionName,
                   symbolName);

            symbolIndex++; // Move to the next symbol
        }

//        for (int i = 0; i < num_symbols; i++) {
//            int section_index = symtab[i].st_shndx;
//            char *section_name = (section_index < header->e_shnum) ? (char *)(map_start + shdr[header->e_shstrndx].sh_offset + shdr[section_index].sh_name) : "N/A";
//            char *symbol_name = &strtab[symtab[i].st_name];
//            printf("[%d]\t0x%x\t%d\t%s\t%s\n",
//                   i, symtab[i].st_value, section_index, section_name, symbol_name);
//        }
        // Print debug information if debug mode is on
        if (debug_mode) {
            printf("Debug: Symbol table size = %lu, Number of symbols = %d\n", (unsigned long)symtab_shdr->sh_size, num_symbols);
            printf("Debug: String table offset = 0x%lx\n", (unsigned long)strtab_shdr->sh_offset);
            printf("Debug: Symbol table offset = 0x%lx\n", (unsigned long)symtab_shdr->sh_offset);
            printf("Debug: Symbol table entry size = %u\n", symtab_shdr->sh_entsize);
            printf("Debug: Symbol table entry count = %d\n", num_symbols);
        }
    }
}
// helper functions: find_symtab() finds the symbol table and returns a pointer to it,
// and count_symbols() counts the number of symbols in the symbol table.
Elf32_Shdr *find_symtab(Elf32_Ehdr *header, void *map_start) {
    Elf32_Shdr *shdr = (Elf32_Shdr *)(map_start + header->e_shoff);
    for (int i = 0; i < header->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB || shdr[i].sh_type == SHT_DYNSYM) {
            return &shdr[i];
        }
    }
    return NULL;
}
// helper functions: get_symbol_name() retrieves a symbol's name given its index,
// and find_symbol() searches for a symbol by name in the given symbol table.
char *get_symbol_name(void *map_start, Elf32_Ehdr *header, Elf32_Sym symbol) {
    Elf32_Shdr *shdr = (Elf32_Shdr *)(map_start + header->e_shoff);
    char *strtab = (char *)(map_start + shdr[header->e_shstrndx].sh_offset);
    return &strtab[symbol.st_name];
}
int count_symbols(Elf32_Shdr *symtab) {
    return symtab->sh_size / symtab->sh_entsize;
}
int find_symbol(char *name, Elf32_Sym *symbols, int num_symbols) {
    for (int i = 0; i < num_symbols; i++) {
        if (strcmp(name, &symbols[i].st_name) == 0) {
            return 1;
        }
    }
    return 0;
}
int find_symbol_index(char *name, Elf32_Sym *symbols, int num_symbols) {
    for (int i = 0; i < num_symbols; i++) {
        if (strcmp(name, &symbols[i].st_name) == 0) {
            return i;
        }
    }
    return -1;
}
void check_files_for_merge() {
    // Check that both ELF files are opened and mapped
    if (map_start1 == NULL || map_start2 == NULL) {
        printf("Error: Two ELF files must be opened and mapped.\n");
        return;
    }

    // Assume helper functions: find_symtab() finds the symbol table and returns a pointer to it,
    // and count_symbols() counts the number of symbols in the symbol table.
    Elf32_Shdr *symtab1 = find_symtab(header1, map_start1);
    Elf32_Shdr *symtab2 = find_symtab(header2, map_start2);

    if (symtab1 == NULL || symtab2 == NULL) {
        printf("Feature not supported: One of the files does not contain a symbol table.\n");
        return;
    }

    // Pointers to the symbol tables
    Elf32_Sym *symbols1 = (Elf32_Sym *)(map_start1 + symtab1->sh_offset);
    Elf32_Sym *symbols2 = (Elf32_Sym *)(map_start2 + symtab2->sh_offset);

    // Assume helper functions: get_symbol_name() retrieves a symbol's name given its index,
    // and find_symbol() searches for a symbol by name in the given symbol table.
    int num_symbols1 = count_symbols(symtab1);
    int num_symbols2 = count_symbols(symtab2);

    // Loop through symbols in the first file
    for (int i = 1; i < num_symbols1; i++) { // Skipping the first dummy symbol
        char *name = get_symbol_name(map_start1, header1, symbols1[i]);
        if (symbols1[i].st_shndx == SHN_UNDEF) { // UNDEFINED symbol
            // Search for the symbol in the second file's symbol table
            if (!find_symbol(name, symbols2, num_symbols2) || symbols2[find_symbol_index(name, symbols2, num_symbols2)].st_shndx == SHN_UNDEF) {
                printf("Symbol %s undefined.\n", name);
            }
        } else { // Defined symbol
            if (find_symbol(name, symbols2, num_symbols2) && symbols2[find_symbol_index(name, symbols2, num_symbols2)].st_shndx != SHN_UNDEF) {
                printf("Symbol %s multiply defined.\n", name);
            }
        }
    }
    // Now check symbols in the second ELF file against those in the first
    for (int i = 1; i < num_symbols2; i++) { // Skipping the first dummy symbol
        char *name = get_symbol_name(map_start2, header2, symbols2[i]);
        if (symbols2[i].st_shndx == SHN_UNDEF) { // UNDEFINED symbol in the second file
            // Search for the symbol in the first file's symbol table
            if (!find_symbol(name, symbols1, num_symbols1) || symbols1[find_symbol_index(name, symbols1, num_symbols1)].st_shndx == SHN_UNDEF) {
                printf("Symbol %s undefined.\n", name);
            }
        } else { // Defined symbol in the second file
            if (find_symbol(name, symbols1, num_symbols1) && symbols1[find_symbol_index(name, symbols1, num_symbols1)].st_shndx != SHN_UNDEF) {
                printf("Symbol %s multiply defined.\n", name);
            }
        }
    }
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

