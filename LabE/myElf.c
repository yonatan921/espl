#include <stdio.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

int debug_mode = 0;//0 for off, 1 for on
int fd1=-1,fd2=-1;//file descriptor
int amount_of_symbols1=-1 ,amount_of_symbols2=-1;
void* map_start1=NULL,*map_start2=NULL; //pointer to the mapped file
struct stat stat1, stat2;
Elf32_Ehdr *header1 = NULL ,*header2 = NULL;


void examine_elf_file();
void print_section_names();
void print_symbols();
void print_symbol(Elf32_Shdr *sh,Elf32_Sym *SYMTAB_symTable,Elf32_Sym *DYNSYM_symTable,char *symbolStringTable,int num_symbols_DYNSYM,int num_symbols_SYMTAB,char *section_header_string_table,Elf32_Ehdr *elf_header,void *map_start);
void check_files_for_merge();
void merge_elf_files();
void quit_program();
void toggle_debug_mode();
void print_examine_inf(int num);
char *getSHType(int num);
void printSection(void *map_start,Elf32_Ehdr *elf_header);
void PrintSymbolsHelper(void *map_start,Elf32_Ehdr *elf_header, int numSymbols);


struct fun_desc {//struct for the menu
    char *name;
    void (*func)();
};


void toggle_debug_mode() {
    debug_mode = !(debug_mode);
    printf("Debug flag now %s\n", debug_mode ? "ON" : "OFF");
}

//func purpose: print the information of the ELF file
void print_examine_inf(int num){
    if(num==1){
        printf("Magic number: %c%c%c\n", header1->e_ident[EI_MAG1], header1->e_ident[EI_MAG2], header1->e_ident[EI_MAG3]);
        printf("The data encoding scheme of the object file: %s\n", (header1->e_ident[EI_DATA] == ELFDATA2LSB) ? "little endian" : (header1->e_ident[EI_DATA] == ELFDATA2MSB) ? "big endian" : "unknown");
        printf("Entry point (hexadecimal address): 0x%x\n", header1->e_entry);
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
        printf("Entry point (hexadecimal address): 0x%x\n", header2->e_entry);
        printf("The file offset in which the section header table resides: %lu\n", (unsigned long) header2->e_shoff);
        printf("The number of section header entries: %u\n", header2->e_shnum);
        printf("The size of each section header entry: %u\n", header2->e_shentsize);
        printf("The file offset in which the program header table resides: %u\n", header2->e_phoff);
        printf("The number of program header entries: %u\n", header2->e_phnum);
        printf("The size of each program header entry: %u\n", header2->e_phentsize);
    }
}

void examine_elf_file(){

    printf("Please enter the file name:\n");
    char buffer[100];
    char file_name[100];
    fgets(buffer, 100, stdin);
    sscanf(buffer, "%s", file_name);


    if (fd1 != -1 &&fd2 != -1) {
        printf("Error: Cannot examine ELF file - already two ELF files.\n");
        return;
    }

    if (fd1==-1){
        fd1 = open(file_name, O_RDWR);
        if (fd1 == -1) {
            printf("Error: cannot open file %s\n", file_name);
            return;
        }
       //The fstat function usually takes a file descriptor as an argument and fills a structure with the file's information.
        if (fstat(fd1, &stat1) == -1) {
            printf("Error getting files information\n");
            close(fd1);
            fd1 = -1;
            return;
        }
      //  mmap() creates a new mapping in the virtual address space of the calling process.
        map_start1 = mmap(NULL, stat1.st_size, PROT_READ, MAP_PRIVATE, fd1, 0);
        if (map_start1 == MAP_FAILED) {
            printf("Error: Error mapping file into the memory.\n");
            close(fd1);
            fd1 = -1;
            return;
        }

        header1 = (Elf32_Ehdr*) map_start1;

        print_examine_inf(1);

        Elf32_Shdr * sh1 = (Elf32_Shdr *)(map_start1 + header1->e_shoff);
        amount_of_symbols1=0;
        int sectionIndex1 = 0;
        while(sectionIndex1 < header1->e_shnum){ // Loop condition
            if(sh1[sectionIndex1].sh_type == SHT_SYMTAB){
                amount_of_symbols1 = sh1[sectionIndex1].sh_size / sh1[sectionIndex1].sh_entsize;
            }
            sectionIndex1++; // Increment the loop variable
        }


    }
    else
    {
        fd2 = open(file_name, O_RDWR);
        if (fd2 == -1) {
            printf("Error: cannot open file %s\n", file_name);
            return;
        }
        //The fstat function usually takes a file descriptor as an argument and fills a structure with the file's information.
        if (fstat(fd2, &stat2) == -1) {
            printf("Error: getting files information\n");
            close(fd2);
            fd2 = -1;
            return;
        }

        map_start2 = mmap(NULL, stat2.st_size, PROT_READ, MAP_PRIVATE, fd2, 0);
        if (map_start2 == MAP_FAILED) {
            printf("Error: Error mapping file into the memory.\n");
            close(fd2);
            fd2 = -1;
            return;
        }

        header2 = (Elf32_Ehdr*) map_start2;

        print_examine_inf(2);

        Elf32_Shdr * sh2 = (Elf32_Shdr *)(map_start2 + header2->e_shoff);
        amount_of_symbols2=0;
        int sectionIndex = 0;
        while(sectionIndex < header2->e_shnum){ // Loop condition

            if(sh2[sectionIndex].sh_type == SHT_SYMTAB){
                amount_of_symbols2 = sh2[sectionIndex].sh_size / sh2[sectionIndex].sh_entsize;
            }

            sectionIndex++; // Increment the loop variable
        }

    }


}
char *getSHType(int type) { // Get section header type
    switch (type) {
        case SHT_NULL:
            return "NULL";
        case SHT_PROGBITS:
            return "PROGBITS";
        case SHT_NOTE:
            return "NOTE";
        case SHT_NOBITS:
            return "NOBITS";
        case SHT_REL:
            return "REL";
        case SHT_SHLIB:
            return "SHLIB";
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
        case SHT_FINI_ARRAY:
            return "FINI_ARRAY";
        case SHT_PREINIT_ARRAY:
            return "PREINIT_ARRAY";
        case SHT_GROUP:
            return "GROUP";
        case SHT_SYMTAB_SHNDX:
            return "SYMTAB_SHNDX";
        case SHT_DYNSYM:
            return "DYNSYM";
        case SHT_INIT_ARRAY:
            return "INIT_ARRAY";
        default:
            return "UNKNOWN";
    }
}

void print_section_names(){
    if (fd1 == -1 && fd2 == -1) {
        printf("Error: Cannot print section names - no ELF files.\n");
        return;
    }

    for (int filenum = 1; filenum <= 2; filenum++) {
        // Determine which file to work with
        Elf32_Ehdr *header = (filenum == 1) ? header1 : header2;
        void *map_start = (filenum == 1) ? map_start1 : map_start2;
        //struct stat sb = (filenum == 1) ? sb1 : sb2;
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
                   getSHType(shdr[section_header].sh_type));
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
    if (fd1 == -1 && fd2 == -1) {
        printf("Error: Couldn't print symbols - no ELF files.\n");
        return;
    }
    if (fd1 != -1) {
        Elf32_Shdr *sh = (Elf32_Shdr *) (map_start1 + header1->e_shoff);
        Elf32_Sym *SYMTAB_symTable = NULL;
        Elf32_Sym *DYNSYM_symTable = NULL;
        char *symbolStringTable = NULL;
        int num_symbols_DYNSYM = 0;
        int num_symbols_SYMTAB = amount_of_symbols1;
        char *section_header_string_table = (char *) (map_start1 + sh[header1->e_shstrndx].sh_offset);
        print_symbol(sh,SYMTAB_symTable,DYNSYM_symTable,symbolStringTable,num_symbols_DYNSYM,num_symbols_SYMTAB,section_header_string_table,header1,map_start1);
    }
    if(fd2 != -1){
        Elf32_Shdr *sh = (Elf32_Shdr *) (map_start2 + header2->e_shoff);
        Elf32_Sym *SYMTAB_symTable = NULL;
        Elf32_Sym *DYNSYM_symTable = NULL;
        char *symbolStringTable = NULL;
        int num_symbols_DYNSYM = 0;
        int num_symbols_SYMTAB = amount_of_symbols2;
        char *section_header_string_table = (char *) (map_start2 + sh[header2->e_shstrndx].sh_offset);
        print_symbol(sh,SYMTAB_symTable,DYNSYM_symTable,symbolStringTable,num_symbols_DYNSYM,num_symbols_SYMTAB,section_header_string_table,header2,map_start2);
    }
}

void print_symbol(Elf32_Shdr *sh,Elf32_Sym *SYMTAB_symTable,Elf32_Sym *DYNSYM_symTable,char *symbolStringTable,int num_symbols_DYNSYM,int num_symbols_SYMTAB,char *section_header_string_table,Elf32_Ehdr *elf_header,void *map_start){
    int sectionHeaderIndex = 0;
    while(sectionHeaderIndex < elf_header->e_shnum){
        if(strcmp(section_header_string_table + sh[sectionHeaderIndex].sh_name, ".strtab") == 0){
            symbolStringTable = (char *)(map_start + sh[sectionHeaderIndex].sh_offset);
        }
        if(sh[sectionHeaderIndex].sh_type == SHT_SYMTAB){
            SYMTAB_symTable = (Elf32_Sym *)(map_start + sh[sectionHeaderIndex].sh_offset);
            num_symbols_SYMTAB = sh[sectionHeaderIndex].sh_size / sh[sectionHeaderIndex].sh_entsize;
        }
        if(sh[sectionHeaderIndex].sh_type == SHT_DYNSYM){
            DYNSYM_symTable = (Elf32_Sym *)(map_start + sh[sectionHeaderIndex].sh_offset);
            num_symbols_DYNSYM = sh[sectionHeaderIndex].sh_size / sh[sectionHeaderIndex].sh_entsize;
        }
        sectionHeaderIndex++;
    }
    if (debug_mode){
        printf("Debug: num_symbols_SYMTAB = %d\n", num_symbols_SYMTAB);
        printf("Debug: num_symbols_DYNSYM = %d\n", num_symbols_DYNSYM);
    }
    printf("\n");

    printf("[index]\tvalue\tsection_index\tsection_name\tsymbol_name\n");

    int dynsymSymbolIndex = 0; // Renamed j to dynsymSymbolIndex for clarity
    while(dynsymSymbolIndex < num_symbols_DYNSYM){
        char *symbolName = "";
        if(DYNSYM_symTable[dynsymSymbolIndex].st_info != STT_SECTION){
            symbolName = (char *)(symbolStringTable + (DYNSYM_symTable[dynsymSymbolIndex].st_name));
        }
        int indexForDYNSYM = DYNSYM_symTable[dynsymSymbolIndex].st_shndx; // Renamed for consistency
        char *nameSection = " ";
        if(indexForDYNSYM == SHN_ABS){
            nameSection = "ABS";
        }
        else if(indexForDYNSYM == SHN_UNDEF){
            nameSection = "UND";
        }
        else{
            nameSection = (char *)(section_header_string_table + sh[indexForDYNSYM].sh_name);
        }

        printf("%2d\t%08x\t%5d\t%15s\t%s\n", dynsymSymbolIndex, DYNSYM_symTable[dynsymSymbolIndex].st_value, indexForDYNSYM, nameSection, symbolName);
        dynsymSymbolIndex++;
    }
    printf("[index]\tvalue\tsection_index\tsection_name\tsymbol_name\n");
    int symtabSymbolIndex = 0;
    while(symtabSymbolIndex < num_symbols_SYMTAB){
        int index = SYMTAB_symTable[symtabSymbolIndex].st_shndx;

        char *sym_Name = " ";

        if(SYMTAB_symTable[symtabSymbolIndex].st_info != STT_SECTION){
            sym_Name = (char *)(symbolStringTable + SYMTAB_symTable[symtabSymbolIndex].st_name);
        }

        char *nameSec ;
        if(index == SHN_ABS){
            nameSec = "ABS";
        }
        else if (index == SHN_UNDEF){
            nameSec = "UND";
        }
        else{
            nameSec = (char *)(section_header_string_table + sh[index].sh_name);
        }

        printf("%2d\t%08x\t%5d\t%15s\t%s\n", symtabSymbolIndex, SYMTAB_symTable[symtabSymbolIndex].st_value, index, nameSec, sym_Name);
        symtabSymbolIndex++;
    }

}
void check_files_for_merge(){
    if (fd1 == -1 || fd2 == -1 ) {
        printf("Error : Cannot check files for merge - no two ELF files.\n");
        return;
    }

    Elf32_Shdr *shStringTable1 = (Elf32_Shdr *)((char *)map_start1 + header1->e_shoff + (header1->e_shstrndx * sizeof(Elf32_Shdr)));
    Elf32_Shdr *symTable1 = NULL;
    Elf32_Shdr *stringTable1 = NULL;
    int sectionIndex1 = 1;
    while(sectionIndex1 < header1->e_shnum){
        Elf32_Shdr *entry1 = (Elf32_Shdr *)((char *)map_start1 + header1->e_shoff + (sectionIndex1 * sizeof(Elf32_Shdr)));
        if(entry1->sh_type == SHT_SYMTAB) {
            symTable1 = entry1;
        }else if(entry1->sh_type == SHT_STRTAB) {
            stringTable1 = entry1;
        }

        sectionIndex1++;
    }


    Elf32_Shdr *shStringTable2 = (Elf32_Shdr *)((char *)map_start2 + header2->e_shoff + (header2->e_shstrndx * sizeof(Elf32_Shdr)));
    Elf32_Shdr *symTable2 = NULL;
    Elf32_Shdr *stringTable2 = NULL;
    int sectionIndex2 = 1;
    while(sectionIndex2 < header2->e_shnum){
        Elf32_Shdr *entry2 = (Elf32_Shdr *)((char *)map_start2 + header2->e_shoff + (sectionIndex2 * sizeof(Elf32_Shdr)));
        if(entry2->sh_type == SHT_SYMTAB){
            symTable2 = entry2;
        }else if(entry2->sh_type == SHT_STRTAB) {
            stringTable2 = entry2;
        }

        sectionIndex2++;
    }


    if (shStringTable1 == NULL || shStringTable2 == NULL || symTable1 == NULL || symTable2 == NULL) {
        printf("Error: Cannot check files for merge - missing section headers.\n");
        return;
    }
    Elf32_Sym *SYMTAB1 = (Elf32_Sym*)(map_start1 + symTable1->sh_offset);
    int symIndex1 = 1; // For searching symbols in symTable1
    while(symIndex1 < symTable1->sh_size / sizeof(Elf32_Sym)) {
        Elf32_Sym *symbol1 = &SYMTAB1[symIndex1];
        char *symbol_name1 = (char*) (map_start1 + stringTable1->sh_offset + symbol1->st_name);
        int found = 1; // Initially true, assuming not found

        if (strcmp("", symbol_name1) != 0) {
            if (symbol1->st_shndx == SHN_UNDEF) {
                int symIndex2 = 1; // For searching undefined symbols in symTable2
                while(symIndex2 < symTable2->sh_size / sizeof(Elf32_Sym)) {
                    Elf32_Sym *SYMTAB2 = (Elf32_Sym*)(map_start2 + symTable2->sh_offset);
                    Elf32_Sym *symbol2 = &SYMTAB2[symIndex2];
                    char *symbol_name2 = (char*) (map_start2 + stringTable2->sh_offset + symbol2->st_name);

                    if (strcmp(symbol_name2, symbol_name1) == 0) {
                        found = 0; // Symbol found
                        if ((symbol2->st_shndx == SHN_UNDEF)) {
                            printf("Symbol sym undefined\n");
                            break;
                        }
                    }
                    symIndex2++;
                }
                if (found == 1) {
                    printf("Symbol sym undefined\n");
                }
            } else {
                // Symbol is defined, search in SYMTAB2
                int symIndex2 = 1; // Resetting for searching defined symbols in symTable2
                while(symIndex2 < symTable2->sh_size / sizeof(Elf32_Sym)) {
                    Elf32_Sym *syms2 = (Elf32_Sym*)(map_start2 + symTable2->sh_offset);
                    Elf32_Sym *sym2 = &syms2[symIndex2];
                    char *symbol_name2 = (char*) (map_start2 + stringTable2->sh_offset + sym2->st_name);

                    if ((sym2->st_shndx != SHN_UNDEF)) {
                        if (strcmp(symbol_name2, symbol_name1) == 0) {
                            printf("\"Symbol sym multiply defined\"\n");
                        }
                    }
                    symIndex2++;
                }
            }
        }
        symIndex1++;
    }

}


void merge_elf_files(){
    if (fd1 == -1 || fd2 == -1 ) {
        printf("Error: Cannot merge ELF files - no two ELF files.\n");
        return;
    }
    int size_symtable1,size_symtable2;
    Elf32_Sym *symbol1=NULL;
    Elf32_Sym *symbol2=NULL;
    char* sec_name1;
    char* sec_name2;
    Elf32_Shdr * sh1 = (Elf32_Shdr *)(map_start1 + header1->e_shoff);
    Elf32_Shdr * sh2 = (Elf32_Shdr *)(map_start2 + header2->e_shoff);
    int sectionIndex1 = 0;
    while(sectionIndex1 < header1->e_shnum) {
        if (sh1[sectionIndex1].sh_type == SHT_SYMTAB || sh1[sectionIndex1].sh_type == SHT_DYNSYM) {
            if (symbol1 != NULL) {
                fprintf(stdout, "Error: we do not support more than one symbol table in a file.\n");
                return;
            } else {
                symbol1 = (Elf32_Sym*)(map_start1 + sh1[sectionIndex1].sh_offset);
                sec_name1 = (char*)(map_start1 + sh1[sh1[sectionIndex1].sh_link].sh_offset);
                size_symtable1 = sh1[sectionIndex1].sh_size / sh1[sectionIndex1].sh_entsize;
            }
        }
        sectionIndex1++;
    }

    int sectionIndex2 = 0;
    while(sectionIndex2 < header2->e_shnum) {
        if (sh2[sectionIndex2].sh_type == SHT_SYMTAB || sh2[sectionIndex2].sh_type == SHT_DYNSYM) {
            if (symbol2 != NULL) {
                fprintf(stdout, "Error: we do not support more than one symbol table in a file.\n");
                return;
            } else {
                symbol2 = (Elf32_Sym*)(map_start2 + sh2[sectionIndex2].sh_offset);
                sec_name2 = (char*)(map_start2 + sh2[sh2[sectionIndex2].sh_link].sh_offset);
                size_symtable2 = sh2[sectionIndex2].sh_size / sh2[sectionIndex2].sh_entsize;
            }
        }
        sectionIndex2++;
    }


    FILE* union_output = fopen("out.ro", "wb");
    if (union_output == NULL) {
        fprintf(stderr, "Error: Cannot open file for writing\n");
        return;
    }
    Elf32_Shdr unionsh1[header1->e_shnum];
    fwrite((char*)header1, 1, header1->e_ehsize, union_output);

    // Copy section headers from file1 to a new array
    memcpy((char*)unionsh1, (char*)sh1, header1->e_shnum * header1->e_shentsize);
    // Write sections from file1 to output file and merge relevant sections with file2
    int sectionIndex = 0;
    while(sectionIndex < header1->e_shnum) {
        unionsh1[sectionIndex].sh_offset = ftell(union_output);
        char* comp = (char*)(map_start1 + sh1[header1->e_shstrndx].sh_offset) + sh1[sectionIndex].sh_name;

        if (strcmp(comp, ".shstrtab") == 0) {
            fwrite((char*)(map_start1 + sh1[sectionIndex].sh_offset), 1, sh1[sectionIndex].sh_size, union_output);
        } else if (strcmp(comp, ".rodata") == 0 || strcmp(comp, ".data") == 0 || strcmp(comp, ".text") == 0) {
            fwrite((char*)(map_start1 + sh1[sectionIndex].sh_offset), 1, sh1[sectionIndex].sh_size, union_output);
            Elf32_Shdr* section = NULL;
            // Nested Loop for finding matching section in file2
            int matchingSectionIndex = 0;
            while(matchingSectionIndex < header2->e_shnum) {
                if (strcmp((char*)(map_start2 + sh2[header2->e_shstrndx].sh_offset) + sh2[matchingSectionIndex].sh_name, comp) == 0) {
                    section = &sh2[matchingSectionIndex];
                    break;
                }
                matchingSectionIndex++;
            }

            if (section != NULL) {
                unionsh1[sectionIndex].sh_size += section->sh_size;
                unionsh1[sectionIndex].sh_offset = ftell(union_output); // Adjust the offset before writing additional data
                fwrite((char*)(map_start2 + section->sh_offset), 1, section->sh_size, union_output);
            }
        } else if (strcmp(comp, ".symtab") == 0) {
            fwrite((char*)symbol1, 1, size_symtable1 * sizeof(Elf32_Sym), union_output);
            fwrite((char*)sec_name1, 1, sh1[sh1[sectionIndex].sh_link].sh_size, union_output);
        } else if (strcmp(comp, ".strtab") == 0) {
            fwrite((char*)sec_name1, 1, sh1[sectionIndex].sh_size, union_output);
        }else {
            fwrite((char*)(map_start1 + sh1[sectionIndex].sh_offset), 1, sh1[sectionIndex].sh_size, union_output);
        }

        sectionIndex++;
    }


    // Update the offset in the output file
    const int OFFSET_POSITION = 32;
    int offset = ftell(union_output);

    char offsetBuffer[sizeof(int)];
    memcpy(offsetBuffer, &offset, sizeof(int));

    // Write the offset at the specified position in the file
    fseek(union_output, OFFSET_POSITION, SEEK_SET);
    fwrite(offsetBuffer, sizeof(char), sizeof(int), union_output);

    fclose(union_output);
}


void quit_program() {
    if (fd1 != -1) {
        close(fd1);
    }
    if (fd2 != -1) {
        close(fd2);
    }
    if (map_start1 != NULL) {
        munmap(map_start1, stat1.st_size);
    }
    if (map_start2 != NULL) {
        munmap(map_start2, stat2.st_size);
    }
    exit(EXIT_SUCCESS);
}


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
            printf("Not within bounds\n");
            exit(EXIT_SUCCESS);
        }


    }
}
int main() {
    menu_func();
    return 0;
}

