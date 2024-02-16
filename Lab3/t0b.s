section .text
global _start
global system_call
extern main
section .data
    hello db 'hello world', 0xA ; 'Hello world' and newline character
    len equ $ - hello ; Calculate the length of the string

section .text
    global _start

_t0b:
    mov eax, 4 ; System call number for sys_write
    mov ebx, 1 ; File descriptor 1 (stdout)
    mov ecx, hello ; Pointer to the string
    mov edx, len ; Length of the string
    int 0x80 ; Call kernel

    ; Exit the program
    mov eax, 1 ; System call number for sys_exit
    xor ebx, ebx ; Status 0
    int 0x80 ; Call kernel