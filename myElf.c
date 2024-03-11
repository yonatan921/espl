#include <stdio.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>


//global var

int debug_mode = 0;
int fd1=-1;
int fd2=-1;
void* map_start1=NULL;
void* map_start2=NULL;
struct stat stNum1;
struct stat stNum2;
Elf32_Ehdr *header1 = NULL;
Elf32_Ehdr *header2 = NULL;
int amount_of_symbols1=-1;
int amount_of_symbols2=-1;

//----------------------------------------------------------------------------------//
struct fun_desc {
    char *name;
    void (*action)();
};
//----------------------------------------------------------------------------------//
void ToggleDebugMode(){
    if (debug_mode==0) {
        debug_mode=1;
        printf("Debug: Debug flag now on\n");
    }
    else if (debug_mode==1){
        debug_mode=0;
        printf("Debug: Debug flag now off\n");
    }
}
void SaveNumSymbols1(){
    Elf32_Shdr * section_header = (Elf32_Shdr *)(map_start1 + header1->e_shoff);
    int i;
    amount_of_symbols1=0;
    for(i = 0; i < header1->e_shnum; i++){

        if(section_header[i].sh_type == SHT_SYMTAB){
            amount_of_symbols1 = section_header[i].sh_size / section_header[i].sh_entsize;
        }

    }
}
void SaveNumSymbols2(){
    Elf32_Shdr * section_header = (Elf32_Shdr *)(map_start2 + header2->e_shoff);
    int i;
    amount_of_symbols2=0;
    for(i = 0; i < header2->e_shnum; i++){

        if(section_header[i].sh_type == SHT_SYMTAB){
            amount_of_symbols2 = section_header[i].sh_size / section_header[i].sh_entsize;
        }

    }
}
void OpenAndMap1(char fileName[] ){

    fd1 = open(fileName, O_RDWR);
    if (fd1 == -1) {
        printf("OpenAndMap:Error opening the file: %s\n", fileName);
        return;
    }
    //The fstat function usually takes a file descriptor as an argument and fills a structure with the file's information.
    if (fstat(fd1, &stNum1) == -1) {
        printf("Error getting file size.\n");
        close(fd1);
        fd1 = -1;
        return;
    }
    //The mmap function typically takes parameters such as the file descriptor of the file to be mapped
    map_start1 = mmap(NULL, stNum1.st_size, PROT_READ, MAP_PRIVATE, fd1, 0);
    if (map_start1 == MAP_FAILED) {
        printf("Error mapping the file into memory.\n");
        close(fd1);
        fd1 = -1;
        return;
    }

    header1 = (Elf32_Ehdr*) map_start1;

    // Print the required information from the ELF header
    printf("Bytes 1,2,3 of the magic number: %c%c%c\n", header1->e_ident[1], header1->e_ident[2], header1->e_ident[3]);
    printf("Data encoding scheme: %s\n", (header1->e_ident[EI_DATA] == ELFDATA2LSB) ? "2's complement, little endian" : "2's complement, big endian");
    printf("Entry point: 0x%x\n", header1->e_entry);
    printf("Section header table offset: %u\n", header1->e_shoff);
    printf("Number of section header entries: %u\n", header1->e_shnum);
    printf("Size of each section header entry: %u\n", header1->e_shentsize);
    printf("Program header table offset: %u\n", header1->e_phoff);
    printf("Number of program header entries: %u\n", header1->e_phnum);
    printf("Size of each program header entry: %u\n", header1->e_phentsize);
}
void OpenAndMap2(char fileName[] ){

    fd2 = open(fileName, O_RDWR);
    if (fd2 == -1) {
        printf("OpenAndMap:Error opening the file: %s\n", fileName);
        return;
    }
    //The fstat function usually takes a file descriptor as an argument and fills a structure with the file's information.
    if (fstat(fd2, &stNum2) == -1) {
        printf("Error getting file size.\n");
        close(fd2);
        fd2 = -1;
        return;
    }

    map_start2 = mmap(NULL, stNum2.st_size, PROT_READ, MAP_PRIVATE, fd2, 0);
    if (map_start2 == MAP_FAILED) {
        printf("Error mapping the file into memory.\n");
        close(fd2);
        fd2 = -1;
        return;
    }
    header2 = (Elf32_Ehdr*) map_start2;


    // Print the required information from the ELF header
    printf("Bytes 1,2,3 of the magic number: %c%c%c\n", header2->e_ident[1], header2->e_ident[2], header2->e_ident[3]);
    printf("Data encoding scheme: %s\n", (header2->e_ident[EI_DATA] == ELFDATA2LSB) ? "2's complement, little endian" : "2's complement, big endian");
    printf("Entry point: 0x%x\n", header2->e_entry);
    printf("Section header table offset: %u\n", header2->e_shoff);
    printf("Number of section header entries: %u\n", header2->e_shnum);
    printf("Size of each section header entry: %u\n", header2->e_shentsize);
    printf("Program header table offset: %u\n", header2->e_phoff);
    printf("Number of program header entries: %u\n", header2->e_phnum);
    printf("Size of each program header entry: %u\n", header2->e_phentsize);
}
void ExamineELFFile(){

    printf("Enter the ELF file name:  ");
    char buff[100];
    char filename[100];
    fgets(buff, 100, stdin);
    sscanf(buff, "%s", filename);


    if (fd1 != -1 &&fd2 != -1) {
        printf("ExamineELFFile:Cannot examine more than 2 ELF files.\n");
        return;
    }

    if (fd1==-1){
        OpenAndMap1(filename);
        SaveNumSymbols1();
    }
    else
    {
        OpenAndMap2(filename);
        SaveNumSymbols2();
    }


}
char *getSectionHeaderType(int num)
{
    switch (num)
    {
        case 0:
            return "SHT_NULL";
        case 1:
            return "SHT_PROGBITS";
        case 2:
            return "SHT_SYMTAB";
        case 3:
            return "SHT_STRTAB";
        case 4:
            return "SHT_RELA";
        case 5:
            return "SHT_HASH";
        case 6:
            return "SHT_DYNAMIC";
        case 7:
            return "SHT_NOTE";
        case 8:
            return "SHT_NOBITS";
        case 9:
            return "SHT_REL";
        case 10:
            return "SHT_SHLIB";
        case 11:
            return "SHT_DYNSYM";
        default:
            return "";
    }
}
void printSection(void *map_start,Elf32_Ehdr *elf_header) {
    Elf32_Shdr *section_header = (Elf32_Shdr *)(map_start + elf_header->e_shoff);
    char *shstrtab = (char *)(map_start + section_header[elf_header->e_shstrndx].sh_offset);
    int num_sections = elf_header->e_shnum;
    printf("Section headers:\n");
    printf("[index] nameSection  section_address section_offset section_size  section_type\n");
    int i;
    for (i = 0; i <num_sections; i++)
    {
        char *name = shstrtab + section_header[i].sh_name;
        char* sectionType = getSectionHeaderType(section_header[i].sh_type);
        printf("[%d] %s, %x, %x, %d, %s\n", i, name, section_header[i].sh_addr, section_header[i].sh_offset,
               section_header[i].sh_size, sectionType);
    }

}
void PrintSectionNames(){
    if (fd1 == -1 &&fd2 == -1) {
        printf("PrintSectionNames:Cannot Print Section- no ELF files.\n");
        return;
    }

    if (fd1!=-1){
        printSection(map_start1,header1);
    }
    if (fd2!=-1)
    {
        printSection(map_start2,header2);
    }
}
void PrintSymbolsHelper(void *map_start,Elf32_Ehdr *elf_header, int numSymbols) {

    Elf32_Shdr * section_header = (Elf32_Shdr *)(map_start + elf_header->e_shoff);
    Elf32_Sym * symbolTableForSYMTAB = NULL;
    Elf32_Sym * symbolTableForDYNSYM = NULL;
    int numberOfSymbolsForSYMTAB = numSymbols;
    int numberOfSymbolsForDYNSYM = 0;
    char * symbolStringTable = NULL;
    char *stringTableOfHeader = (char *)(map_start + section_header[elf_header->e_shstrndx].sh_offset);
    int k, j,i;
    for(i = 0; i < elf_header->e_shnum; i++){
        if(strcmp(stringTableOfHeader + section_header[i].sh_name, ".strtab") == 0){
            symbolStringTable = (char *)(map_start + section_header[i].sh_offset);
        }
        if(section_header[i].sh_type == SHT_SYMTAB){
            symbolTableForSYMTAB = (Elf32_Sym * )(map_start + section_header[i].sh_offset);
            numberOfSymbolsForSYMTAB = section_header[i].sh_size/section_header[i].sh_entsize;
        }
        if (section_header[i].sh_type == SHT_DYNSYM){
            symbolTableForDYNSYM = (Elf32_Sym *)(map_start + section_header[i].sh_offset);
            numberOfSymbolsForDYNSYM = section_header[i].sh_size / section_header[i].sh_entsize;
        }
    }
    if (debug_mode){
        printf("Size OF DYNSYM Symbol: %d \n",numberOfSymbolsForDYNSYM);
        printf("Size OF SYMTAB Symbol: %d ",numberOfSymbolsForSYMTAB);
    }
    printf("\n");

    printf("[index]   value   section_index     section_name     symbol_name\n");

    for(j = 0; j < numberOfSymbolsForDYNSYM; j++){

        char *symbolName = "";
        if(symbolTableForDYNSYM[j].st_info != STT_SECTION){
            symbolName = (char *)(symbolStringTable + (symbolTableForDYNSYM[j].st_name));
        }
        int index_For_DYNSYM=symbolTableForDYNSYM[j].st_shndx;
        char * nameSection = " ";
        if(index_For_DYNSYM == SHN_ABS){
            nameSection = "ABS";
        }
        else if (index_For_DYNSYM==SHN_UNDEF){
            nameSection = "UND";
        }
        else{
            nameSection = (char *)(stringTableOfHeader + section_header[index_For_DYNSYM].sh_name);
        }

        printf("%2d      %08x       %5d    %15s    %s   \n", j, symbolTableForDYNSYM[j].st_value, index_For_DYNSYM, nameSection, symbolName);
    }

    printf("[index]   value   section_index     section_name     symbol_name\n");
    for(k = 0; k < numberOfSymbolsForSYMTAB; k++){
        int index = symbolTableForSYMTAB[k].st_shndx;

        char * symbolName = " ";

        if(symbolTableForSYMTAB[k].st_info != STT_SECTION){
            symbolName = (char *)(symbolStringTable + symbolTableForSYMTAB[k].st_name);
        }

        char * nameSection =" ";
        if(index == SHN_ABS){
            nameSection = "ABS";
        }
        else if (index==SHN_UNDEF){
            nameSection = "UND";
        }
        else{
            nameSection = (char *)(stringTableOfHeader + section_header[index].sh_name);
        }

        printf("%2d      %08x       %5d    %15s    %s   \n", i, symbolTableForSYMTAB[k].st_value, index, nameSection, symbolName);
    }

}
void PrintSymbols(){
    if (fd1 == -1 &&fd2 == -1) {
        printf("PrintSymbols :Cannot Print Symbols- no ELF files.\n");
        return;
    }
    if (fd1!=-1){
        PrintSymbolsHelper(map_start1,header1,amount_of_symbols1);
    }
    if (fd2!=-1)
    {
        PrintSymbolsHelper(map_start2,header2,amount_of_symbols2);
    }
}
void CheckFilesForMerge(){
    if (fd1 == -1 || fd2 == -1 ) {
        printf("CheckFilesForMerge :there are no 2 ELF files.\n");
        return;
    }
    int i;
    Elf32_Shdr *sectionHeaderStringTableNum1 = (Elf32_Shdr *)((char *)map_start1 + header1->e_shoff + (header1->e_shstrndx * sizeof(Elf32_Shdr)));
    Elf32_Shdr *symbolTableNum1 = NULL;
    Elf32_Shdr *stringTableNum1 = NULL;

    for ( i = 1; i < header1->e_shnum; i++){
        Elf32_Shdr *entry = (Elf32_Shdr *)((char *)map_start1 + header1->e_shoff + (i * sizeof(Elf32_Shdr)));
        if (entry->sh_type==SHT_SYMTAB){symbolTableNum1 = entry;}
        else if (entry->sh_type==SHT_STRTAB){stringTableNum1 = entry;}

    }
    Elf32_Shdr *sectionHeaderStringTableNum2 = (Elf32_Shdr *)((char *)map_start2 + header2->e_shoff + (header2->e_shstrndx * sizeof(Elf32_Shdr)));
    Elf32_Shdr *symbolTableNum2 = NULL;
    Elf32_Shdr *stringTableNum2 = NULL;

    for ( i = 1; i < header2->e_shnum; i++){
        Elf32_Shdr *entry2 = (Elf32_Shdr *)((char *)map_start2 + header2->e_shoff + (i * sizeof(Elf32_Shdr)));
        if (entry2->sh_type==SHT_SYMTAB){symbolTableNum2 = entry2;}
        else if (entry2->sh_type==SHT_STRTAB){stringTableNum2 = entry2;}
    }

    if (sectionHeaderStringTableNum1 == NULL || sectionHeaderStringTableNum2 == NULL || symbolTableNum1 == NULL || symbolTableNum2 == NULL) {
        printf("CheckFilesForMerge : 2 ELF files have not been opened and mapped.\n");
        return;
    }
    Elf32_Sym *syms = (Elf32_Sym*)(map_start1 + symbolTableNum1->sh_offset);
    for (int i = 1; i < symbolTableNum1->sh_size / sizeof(Elf32_Sym); i++) {
        Elf32_Sym *sym = &syms[i];
        char *symbol_name1 = (char*) (map_start1+ stringTableNum1->sh_offset + sym->st_name);
        int found = 1;//false
        if (strcmp("",symbol_name1 ) !=0){
            if (sym->st_shndx == SHN_UNDEF) {
                for (int j = 1; j < symbolTableNum2->sh_size / sizeof(Elf32_Sym); j++) {
                    Elf32_Sym *syms2 = (Elf32_Sym*)(map_start2 + symbolTableNum2->sh_offset);
                    Elf32_Sym *sym2 = &syms2[j];
                    char *symbol_name2 = (char*) (map_start2+ stringTableNum2->sh_offset + sym2->st_name);
                    if (strcmp(symbol_name2, symbol_name1) == 0){
                        found = 0;
                        if ((sym2->st_shndx == SHN_UNDEF)) {
                            printf("Symbol %s undefined\n",symbol_name1);
                            break;
                        }
                    }
                }
                if (found == 1) {
                    printf("Symbol %s undefined\n",symbol_name1);
                }

            }
            else {
                // Symbol is defined, search in SYMTAB2
                for (int j = 1; j < symbolTableNum2->sh_size / sizeof(Elf32_Sym); j++) {
                    Elf32_Sym *syms2 = (Elf32_Sym*)(map_start2 + symbolTableNum2->sh_offset);
                    Elf32_Sym *sym2 = &syms2[j];
                    char *symbol_name2 = (char*) (map_start2+ stringTableNum2->sh_offset + sym2->st_name);
                    if ((sym2->st_shndx != SHN_UNDEF)) {
                        if (strcmp(symbol_name2, symbol_name1) == 0){
                            printf("Symbol %s multiply defined\n",symbol_name1);
                        }
                    }
                }
            }
        }
    }
}


void MergeELFFiles(){
    if (fd1 == -1 || fd2 == -1 ) {
        printf("MergeELFFiles :there are no 2 ELF files.\n");
        return;
    }
    int s1,s2,i, j;
    Elf32_Sym *sym1=NULL;
    Elf32_Sym *sym2=NULL;
    char* name1;
    char* name2;
    Elf32_Shdr * sectionheader1 = (Elf32_Shdr *)(map_start1 + header1->e_shoff);
    Elf32_Shdr * sectionheader2 = (Elf32_Shdr *)(map_start2 + header2->e_shoff);
    for (i = 0; i < header1->e_shnum; i++) {
        if (sectionheader1[i].sh_type == SHT_SYMTAB || sectionheader2[i].sh_type == SHT_DYNSYM) {
            if (sym1 != NULL) {
                fprintf( stdout,"Error: 2 symbol tables in one file.\n");
                return;
            } else {
                sym1 = (Elf32_Sym*) (map_start1 + sectionheader1[i].sh_offset);
                name1 = (char*) (map_start1 + sectionheader1[sectionheader1[i].sh_link].sh_offset);
                s1 = sectionheader1[i].sh_size / sectionheader1[i].sh_entsize;
            }
        }
    }
    for (j = 0; j < header2->e_shnum; j++) {
        if (sectionheader1[j].sh_type == SHT_SYMTAB || sectionheader2[j].sh_type == SHT_DYNSYM) {
            if (sym2 != NULL) {
                fprintf( stdout,"Error: 2 symbol tables in one file.\n");
                return;
            } else {
                sym2 = (Elf32_Sym*) (map_start2 + sectionheader2[i].sh_offset);
                name2 = (char*) (map_start2 + sectionheader2[sectionheader2[j].sh_link].sh_offset);
                s2 = sectionheader2[j].sh_size / sectionheader1[j].sh_entsize;
            }
        }
    }

    FILE* unionRES = fopen("out.ro", "wb");
    if (unionRES == NULL) {
        fprintf(stderr, "Error: could not create output file. \n");
        return;
    }
    Elf32_Shdr unionSectionheader1[header1->e_shnum];
    fwrite((char*)header1, 1, header1->e_ehsize, unionRES);

    // Copy section headers from file1 to a new array

    memcpy((char*)unionSectionheader1, (char*)sectionheader1, header1->e_shnum * header1->e_shentsize);
    // Write sections from file1 to output file and merge relevant sections with file2
    for (int k = 0; k < header1->e_shnum; k++) {
        unionSectionheader1[k].sh_offset = ftell(unionRES);
        char* comp = (char*)(map_start1 + sectionheader1[header1->e_shstrndx].sh_offset) + sectionheader1[k].sh_name;

        if (strcmp(comp, ".shstrtab") == 0) {
            fwrite((char*)(map_start1 + sectionheader1[k].sh_offset), 1, sectionheader1[k].sh_size, unionRES);
        } else if (strcmp(comp, ".rodata") == 0 || strcmp(comp, ".data") == 0 || strcmp(comp, ".text") == 0) {
            fwrite((char*)(map_start1 + sectionheader1[k].sh_offset), 1, sectionheader1[k].sh_size, unionRES);
            Elf32_Shdr* section = NULL;

            // Find matching section in file2
            for (int m = 0; m < header2->e_shnum; m++) {
                if (strcmp((char*)(map_start2 + sectionheader2[header2->e_shstrndx].sh_offset) + sectionheader2[m].sh_name, comp) == 0) {
                    section = &sectionheader2[m];
                    break;
                }
            }

            if (section != NULL) {
                unionSectionheader1[k].sh_size += section->sh_size;
                unionSectionheader1[k].sh_offset += section->sh_offset;
                fwrite((char*)(map_start2 + section->sh_offset), 1, section->sh_size, unionRES);
            }
        } else if (strcmp(comp, ".symtab") == 0) {
            //We not supporting in that part
        } else {
            fwrite((char*)(map_start2 + sectionheader1[k].sh_offset), 1, sectionheader1[k].sh_size, unionRES);
        }
    }

    // Update the offset in the output file
    const int OFFSET_POSITION = 32;
    int offset = ftell(unionRES);

    char offsetBuffer[sizeof(int)];
    memcpy(offsetBuffer, &offset, sizeof(int));

    // Write the offset at the specified position in the file
    fseek(unionRES, OFFSET_POSITION, SEEK_SET);
    fwrite(offsetBuffer, sizeof(char), sizeof(int), unionRES);

    fclose(unionRES);
}


void Quit(){

    if (map_start1!=NULL){
        munmap(map_start1,0);
        map_start1=NULL;
    }
    if (fd1!=-1){
        close(fd1);
        fd1=-1;
    }
    if (header1!=NULL){
        header1=NULL;
    }

    if (map_start2!=NULL){
        munmap(map_start2,0);
        map_start2=NULL;
    }
    if (fd2!=-1){
        close(fd1);
        fd2=-1;
    }
    if (header2!=NULL){
        header2=NULL;
    }
    if (debug_mode==1){
        printf("Debug:Exiting...\n");
    }

    exit(0);
}

int main(int argc, char **argv){

    int i,c;
    char out[10];
    struct fun_desc menu[] = {
            { "Toggle Debug Mode", ToggleDebugMode },
            { "Examine ELF File",ExamineELFFile },
            {"Print Section Names",PrintSectionNames},
            {"Print Symbols",PrintSymbols},
            {"Check Files for Merge", CheckFilesForMerge },
            {"Merge ELF Files",MergeELFFiles},
            {"Quit",Quit},
            { NULL, NULL } };
    while (1){
        printf("\nChoose action:\n");
        for( i=0; i < 7;i++){
            printf("%d)",i);
            printf("%s",menu[i].name);
            printf("\n");
        }
        printf("Option :");
        if (fgets(out,10,stdin)==NULL){
            return 0;
        }
        if (memcmp(out,"\n ",1) !=0 &&memcmp(out,"",1)!=0){
            printf("\n");
            c=atoi(out);
            if (c >=0 && c<=9 ){
                menu[c].action();
            }
            else{
                fprintf(stderr,"Not within bounds\n");
            }
        }
    }
    return 0;
}