#include <stdio.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

extern int startup(int argc, char **argv, void (*start)());
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg);
void print_func(Elf32_Phdr *entry,int n);
const char* convertType(int type);
const char* convertFlg(int flg);
int convertFlg2(int flg);
void load_phdr(Elf32_Phdr *phdr, int fd);
int LoadFile(char *filename);
void print_func(Elf32_Phdr *entry,int n);
Elf32_Ehdr *header;
void* map_start; /* will point to the start of the memory mapped file */
struct stat fd_stat; /* this is needed to  the size of the file */
char* currentFilenameOpen=NULL;
int Currentfd = -1;
int mask_page_size = 0xfffff000;


int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg){
    size_t n = 0;
    while (n < header->e_phnum ){
        Elf32_Phdr* entry = map_start + header->e_phoff + (n * header->e_phentsize);
        func(entry,arg);
        n++;
    }
    return 0;
}

const char* convertType(int type) {
    if (type == PT_NULL) {
        return "NULL";
    } else if (type == PT_LOAD) {
        return "LOAD";
    } else if (type == PT_DYNAMIC) {
        return "DYNAMIC";
    } else if (type == PT_INTERP) {
        return "INTERP";
    } else if (type == PT_NOTE) {
        return "NOTE";
    } else if (type == PT_GNU_EH_FRAME) {
        return "GNU_EH_FRAME";
    } else if (type == PT_GNU_STACK) {
        return "GNU_STACK";
    } else if (type == PT_GNU_RELRO) {
        return "GNU_RELRO";
    } else if (type == PT_SUNWBSS) {
        return "SUNWBSS";
    } else if (type == PT_SUNWSTACK) {
        return "SUNWSTACK";
    } else if (type == PT_HIOS) {
        return "HIOS";
    } else if (type == PT_LOPROC) {
        return "LOPROC";
    } else if (type == PT_HIPROC) {
        return "HIPROC";
    } else if (type == PT_SHLIB) {
        return "SHLIB";
    } else if (type == PT_PHDR) {
        return "PHDR";
    } else if (type == PT_TLS) {
        return "TLS";
    } else if (type == PT_NUM) {
        return "NUM";
    } else {
        return "Unknown";
    }
}

const char* convertFlg(int flg) {
    if (flg == 0x000) {
        return "";
    } else if (flg == 0x001) {
        return "E";
    } else if (flg == 0x002) {
        return "W";
    } else if (flg == 0x003) {
        return "WE";
    } else if (flg == 0x004) {
        return "R";
    } else if (flg == 0x005) {
        return "RE";
    } else if (flg == 0x006) {
        return "RW";
    } else if (flg == 0x007) {
        return "RWE";
    } else {
        return "Unknown";
    }
}

int convertFlg2(int flg) {
    switch (flg) {
        case 0x000: return 0;
        case 0x001: return PROT_EXEC;
        case 0x002: return PROT_WRITE;
        case 0x003: return PROT_EXEC | PROT_WRITE;
        case 0x004: return PROT_READ;
        case 0x005: return PROT_READ | PROT_EXEC;
        case 0x006: return PROT_READ | PROT_WRITE;
        case 0x007: return PROT_READ | PROT_WRITE | PROT_EXEC;
        default: return -1;
    }
}

void load_phdr(Elf32_Phdr *phdr, int fd){
    if(phdr -> p_type != PT_LOAD)
        return;
    void *vadddr = (void *)(phdr -> p_vaddr&mask_page_size); //* virtual address of the first byte when it's loaded to memory *//
    int converted_flag = convertFlg2(phdr -> p_flags);
    int off = phdr -> p_offset&0xfffff000;
    int padding_num = phdr -> p_vaddr & 0xfff;
    void* tmp;
    if ((tmp = mmap(vadddr, phdr -> p_memsz+padding_num, converted_flag, MAP_FIXED | MAP_PRIVATE, fd, off)) == MAP_FAILED ) {
        perror("mmap failed1");
        exit(-4);
    }
    print_func(phdr, 0);
}

int LoadFile(char *filename){
    int file_descriptor;
    if((file_descriptor = open(filename, O_RDWR)) < 0) {
        perror("error in open");
        exit(-1);
    }
    if(fstat(file_descriptor, &fd_stat) != 0 ) {
        perror("stat failed");
        exit(-1);
    }
    if ((map_start = mmap(0, fd_stat.st_size, PROT_READ | PROT_WRITE , MAP_SHARED, file_descriptor, 0)) == MAP_FAILED ) {
        perror("mmap failed");
        exit(-4);
    }
    if(Currentfd!=-1)
        close(Currentfd);
    Currentfd=file_descriptor;
    strcpy((char*)&currentFilenameOpen,(char*)filename);
    return Currentfd;
}

void print_func(Elf32_Phdr *entry,int n){
    if(entry->p_type == PT_LOAD){
        printf("%s\t\t%#08x\t%#08x\t%#10.08x\t%#07x\t%#07x\t%s\t%#-6.01x",
               convertType(entry->p_type),entry->p_offset,entry->p_vaddr,entry->p_paddr,entry->p_filesz,entry->p_memsz,convertFlg(entry->p_flags),entry->p_align);
        int converted_flag = convertFlg2(entry -> p_flags);
        printf("   flag : %d", converted_flag);

        printf("\n");


    }else{
        printf("%s\t\t%#08x\t%#08x\t%#10.08x\t%#07x\t%#07x\t%s\t%#-6.01x\n",
               convertType(entry->p_type),entry->p_offset,entry->p_vaddr,entry->p_paddr,entry->p_filesz,entry->p_memsz,convertFlg(entry->p_flags),entry->p_align);
    }
}

int main(int argc, char ** argv){
    if(argc < 2){
        printf("need file");
        exit(1);
    }
    Currentfd = LoadFile(argv[1]);
    header = (Elf32_Ehdr *) map_start;
    foreach_phdr(map_start, load_phdr, Currentfd);
    startup(argc-1, argv+1, (void *)(header->e_entry));
}
