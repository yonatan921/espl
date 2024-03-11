#include <stdio.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>

void print_phdr_info(Elf32_Phdr *phdr, int index) {
    printf("Program header number %d at address %p\n", index, (void*)phdr);
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    if (map_start == MAP_FAILED) return -1;

    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + header->e_phoff);
    for (int i = 0; i < header->e_phnum; ++i) {
        func(&phdr[i], i);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    foreach_phdr(mmap(0, 1000, PROT_READ, MAP_SHARED, open(argv[1], O_RDONLY), 0), print_phdr_info, 0);
    return 0;
}
