section .data
    infile db 0
    outfile db 1
    buf db 1
    newLine db 10

section .text
    global _start

STDOUT EQU 1
EXIT EQU 1
WRITE EQU 4
READ EQU 3
OPEN EQU 5

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop

main:

    call    parse_args
    call    read_input



parse_args:
    
    cmp    ebx, ecx  ; finish read args
    jge    read_input

    mov    edx, [esi + ebx * 4]  ; Get argument
    cmp    byte [edx], '-'       
    jne    next_arg

    mov    al, byte [edx + 1]    
    call check_flags

read_input:
    ; Read input character
    mov    eax, READ      ; Read
    movzx    ebx, byte[infile]    ; Set infile
    mov    ecx, buf ; buf address
    mov    edx, 1      ; Read 1 byte
    int    0x80        ; Call the kernel

    ; Check for end of input
    cmp    eax, 0      ; 
    je     exit_program

    ; Perform encoding
    mov    dl, byte [buf]
    cmp    dl, 'A'
    jl     print_character
    cmp    dl, 'z'
    jg     print_character
    cmp    dl, 'z'
    je     change_z   
    cmp    dl, 'Z'
    je     change_z  
    inc    dl      ; Encode 




print_character:
    ; Write encoded character
    mov byte [buf], dl  ; 
    mov    eax, WRITE     ; Write
    movzx  ebx, byte[outfile]      ; Set outfile
    mov    ecx, buf ; buf address
    mov    edx, 1      ; 
    int    0x80        ; 
    jmp    read_input  ; 

        

next_arg: ; move to the next arg and print the current one
    push ebx
    push ecx
    push edx          ; Push the address of the string
    call strlen       ; get len of the string
    add esp, 4        ; Adjust the stack pointer
    mov edi, eax
    mov    eax, 4
    mov    ebx, 1
    mov    ecx, edx ; 
    mov    edx, edi       ; Write all the argument 
    int    0x80        ; 
    call print_newline
    pop ecx
    pop ebx
    inc    ebx        ; Move to the next argument
    jmp    parse_args

exit_program:
    ; Exit program
    mov    eax, EXIT     ; System call number 1 is exit
    xor    ebx, ebx    ; Exit with return code 0
    int    0x80        ; 

change_z:
    sub dl,25
    jmp print_character

check_flags:
    ;check flags
    cmp    al, 'i'
    je    set_infile ; 
    cmp    al, 'o'
    je    set_outfile ; 
    jmp    next_arg


set_infile:
    push eax
    push edx
    push ebx
    push ecx
    lea ebp, [edx+2]; 
    mov    eax, OPEN       ; Open
    mov    ebx, ebp; File name or path
    mov    ecx, 0       ; read only
    int    0x80         ; 
    mov [infile], eax ; Set infile
    call print_newline
    pop ecx
    pop ebx 
    pop edx
    pop eax
    jmp next_arg

set_outfile:
    push edx
    push ebx
    push ecx
    lea ebp, [edx+2]; 
    mov    ecx, 1 | 64 | 512      ; Write Only
    mov    eax, OPEN       ; Open
    mov    ebx, ebp; File name or path
    mov     edx, 0644 ;
    int    0x80         ;
    mov [outfile], eax ; Set outfile
    call print_newline
    pop ecx
    pop ebx 
    pop edx
    
    jmp next_arg

strlen: 
    mov eax,1
    push edx
    cont: cmp byte [edx], 0
    jz done
    inc edx
    inc eax
    jmp cont
done:
    pop edx 
    ret


print_newline:
    mov eax, WRITE
    mov ebx, STDOUT
    mov ecx, newLine ; buf address
    mov edx, 1       ; Write 1 byte
    int 0x80         ; 
    ret  


system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller



