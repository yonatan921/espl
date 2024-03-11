#include <stdio.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

extern int startup(int argc, char ** argv, void (*start)());
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg);
char* concatenateStrings(const char* str1, const char* str2);
char *getpType(Elf32_Word i);
void print_phdr_info(Elf32_Phdr *phdr, int arg);
void load_phdr(Elf32_Phdr *phdr, int fd);
int getProtFlags(Elf32_Phdr *phdr);


int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    if (map_start == MAP_FAILED) return -1;

    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + header->e_phoff);
    for (int i = 0; i < header->e_phnum; ++i) {
        func(&phdr[i], arg);
    }
    return 0;
}

char* concatenateStrings(const char* str1, const char* str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int totalLen = len1 + len2 + 1; // +1 for the null terminator
    char* result = (char*)malloc(totalLen * sizeof(char));
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}
char *getpType(Elf32_Word i)
{
    if (i== PT_NULL){return "NULL";}
    else if(i==PT_LOAD){return "LOAD";}
    else if(i== PT_DYNAMIC){return "DYNAMIC";}
    else if(i==PT_INTERP){return "INTERP";}
    else if(i==PT_NOTE){return "NOTE";}
    else if(i==PT_SHLIB){return "SHLIB";}
    else if(i==PT_PHDR){return "PHDR";}
    else if(i==PT_TLS){return "TLS";}
    else if(i==PT_NUM){ return "NUM";}
    else if(i==PT_GNU_EH_FRAME){return "GNU_EH_FRAME";}
    else if(i==PT_GNU_STACK){return "GNU_STACK";}
    else if(i==PT_GNU_RELRO){return "GNU_RELRO";}
    else if(i==PT_SUNWBSS){return "SUNWBSS";}
    else if(i==PT_SUNWSTACK){return "SUNWSTACK";}
    else if(i==PT_HIOS){return "HIOS";}
    else if(i==PT_LOPROC){return "LOPROC";}
    else if(i==PT_HIPROC){return "HIPROC";}
    else if(i==PT_INTERP){return "INTERP";}
    else{ return

void print_phdr_info(Elf32_Phdr *phdr, int arg)
{
    char *type = getpType(phdr->p_type);
    char *E = "PROT_EXEC";
    char *W = "PROT_WRITE";
    char *R = "PROT_READ";
    char *MAP = "MAP_PRIVATE | MAP_FIXED";
    char *OR = " | ";

    char flags[4] = {' ', ' ', ' ', '\0'};
    char *protFlags=" ";
    int coun = 0;

    if (phdr->p_flags & PF_R) {
        flags[0] = 'R';
        protFlags=concatenateStrings(protFlags,R);
        coun++;
    }
    if (phdr->p_flags & PF_W) {
        flags[1] = 'W';
        if (coun == 1){protFlags=concatenateStrings(protFlags,OR);}
        protFlags=concatenateStrings(protFlags,W);
        coun++;

    }
    if (phdr->p_flags & PF_X) {
        flags[2] = 'X';
        if (coun > 0){protFlags=concatenateStrings(protFlags,OR);}
        protFlags=concatenateStrings(protFlags,E);

    }


    if(phdr->p_type == PT_LOAD)
    {
        printf("%-7s 0x%06x  0x%08x  0x%08x  0x%06x  0x%06x  %-3s  0x%x %s %s\n",type, phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, flags, phdr->p_align,protFlags,MAP);
    }

}

int getProtFlags(Elf32_Phdr *phdr)
{
    int prot_flags = 0;

    if (phdr->p_flags & PF_R) {prot_flags += PROT_READ;}
    if (phdr->p_flags & PF_W) {prot_flags += PROT_WRITE;}
    if (phdr->p_flags & PF_X) {prot_flags += PROT_EXEC;}

    return prot_flags;
}

void load_phdr(Elf32_Phdr *phdr, int fd)
{
    if(phdr->p_type == PT_LOAD)
    {
        int vaddr = phdr->p_vaddr&0xfffff000;
        int offset = phdr->p_offset&0xfffff000;
        int padding = phdr->p_vaddr&0xfff;
        mmap((void*)vaddr, phdr->p_memsz + padding, getProtFlags(phdr), MAP_FIXED | MAP_PRIVATE , fd, offset);
        print_phdr_info(phdr,fd);
    }

}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("The argument not contain the name of a 32bit ELF.\n");
        return 1;
    }

    char *filename = argv[1];
    int fd = open(filename,O_RDWR);
    if (fd == -1)
    {
        perror("Error opening file.\n");
        return 1;
    }

    struct stat file_stats;
    if (fstat(fd, &file_stats) == -1)
    {
        perror("Error getting file size.\n");
        close(fd);
        return 1;
    }

    void *map_start = mmap(NULL, file_stats.st_size, PROT_READ , MAP_PRIVATE , fd, 0);

    if (map_start == MAP_FAILED)
    {
        perror("Error mapping file to memory");
        close(fd);
        return 1;
    }

    printf("\nType     Offset   VirtAddr    PhysAddr    FileSiz    MemSiz   Flag  Align         ProtectionFlag      MapFlag  \n");

    //task1a
    // foreach_phdr(map_start, print_phdr_info, 0);

    //task2a
    //  the linking script ensures that the loader and the loaded program are mapped to different memory spaces, avoiding clashes.

    //task2b
    // foreach_phdr(map_start, load_phdr, fd);

    //task2c
    foreach_phdr(map_start, load_phdr, fd);
    Elf32_Ehdr * header = (Elf32_Ehdr *) map_start;
    startup(argc-1, argv+1, (void *)(header->e_entry));
    close(fd);

    return 0;
}