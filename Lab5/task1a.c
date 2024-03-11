
#include <stdio.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>


# define buffLen 100
Elf32_Ehdr *header;
void* map_start; //point to the start of the memory mapped file
struct stat fd_stat; //size of file
char* current_file=NULL;
int Currentfd = -1;


char* convertType(int type){
    switch (type){
        case PT_NULL: return "NULL";
        case PT_LOAD: return "LOAD";
        case PT_DYNAMIC: return "DYNAMIC";
        case PT_INTERP: return "INTERP";
        case PT_NOTE: return "NOTE";
        case PT_SHLIB: return "SHLIB";
        case PT_PHDR: return "PHDR";
        case PT_TLS: return "TLS";
        case PT_NUM: return "NUM";
        case PT_GNU_EH_FRAME: return "GNU_EH_FRAME";
        case PT_GNU_STACK: return "GNU_STACK";
        case PT_GNU_RELRO: return "GNU_RELRO";
        case PT_SUNWBSS: return "SUNWBSS";
        case PT_SUNWSTACK: return "SUNWSTACK";
        case PT_HIOS: return "HIOS";
        case PT_LOPROC: return "LOPROC";
        case PT_HIPROC: return "HIPROC";
        default:return "Unknown";
    }
}

char* convertFlg(int flg){
    switch (flg){
        case 0x004: return "R";
        case 0x005: return "R E";
        case 0x006: return "RW";
        default:return "Unknown";
    }
}



int LoadFile(char *filename){
    int fd;
    if((fd = open(filename, O_RDWR)) < 0) {
        perror("error in open");
        exit(-1);
    }
    if(fstat(fd, &fd_stat) != 0 ) {
        perror("stat failed");
        exit(-1);
    }
    if ((map_start = mmap(0, fd_stat.st_size, PROT_READ | PROT_WRITE , MAP_SHARED, fd, 0)) == MAP_FAILED ) {
        perror("mmap failed");
        exit(-4);
    }
    if(Currentfd!=-1)
        close(Currentfd);
    Currentfd=fd;
    strcpy((char*)&current_file,(char*)filename);
    return Currentfd;
}

void print_phdr_info(Elf32_Phdr *entry,int n){
    printf("%s\t\t%#08x\t%#08x\t%#10.08x\t%#07x\t%#07x\t%s\t%#-6.01x\n",
           convertType(entry->p_type),entry->p_offset,entry->p_vaddr,entry->p_paddr,entry->p_filesz,entry->p_memsz,convertFlg(entry->p_flags),entry->p_align);
}
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    if (map_start == MAP_FAILED) return -1;

    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + header->e_phoff);
    printf("Type\t\tOffset\t\tVirtAddr\tPhysAddr\tFileSiz\t\tMemSiz\t\tFlg\tAlign\n");
    for (int i = 0; i < header->e_phnum; ++i) {
        func(&phdr[i], i);
    }
    return 0;
}

int main(int argc, char ** argv){
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    Currentfd = LoadFile(argv[1]);
    header = (Elf32_Ehdr *) map_start;
    // printProgramHeaders();
    foreach_phdr(map_start, print_phdr_info, 0);

}