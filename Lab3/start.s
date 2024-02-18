section .data
    inFile db 0
    outFile db 1
    newline db 10
    buffer db 1

section .text
    global _start

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi, esp    ; esi = argv

    ; Parse command-line arguments
    mov    ebx, 1      ; Argument index starts at 1

    get_arguments:
        cmp    ebx, ecx  ; Check if all arguments have been processed
        jge    read
        mov    edx, [esi + ebx * 4]  ; Get the pointer to the argument
        cmp    byte [edx], '-'       ; Check if the argument starts with '-'
        jne    next_arg

        mov    al, byte [edx + 1]    ; Get the argument flag

        ; Check if it is the -i flag
        cmp    al, 'i'
        je    set_inFile ; 
        cmp    al, 'o'
        je    set_outFile ; 
        jmp    next_arg

    

    read:
        ; Read input character
        mov    edx, 1      ; Read 1 byte
        mov    ecx, buffer ; Buffer address
        mov    eax, 3      ; System call number 3 is read
        int    0x80        ; Call the kernel
        movzx    ebx, byte[inFile]    ; File descriptor. 0 if std in.

        ; Check for end of input
        cmp    eax, 0      ; If zero bytes read, end of input reached
        je     exit_program

        ; Perform encoding
        mov    dl, byte [buffer]
        cmp    dl, 'A'
        jl     print_character
        cmp    dl, 'z'
        jg     print_character
        cmp    dl, 'z'
        je     change_z   
        cmp    dl, 'Z'
        je     change_z  
        inc    dl      ; Encode character by adding 1
  



    print_character:
        ; Write encoded character
        mov    edx, 1      ; Write 1 byte
        mov byte [buffer], dl  ; Move the value from al back to buffer
        mov    eax, 4      ; System call number 4 is write
        movzx  ebx, byte[outFile]      ; File descriptor 1 is stdout
        mov    ecx, buffer ; Buffer address
        
        int    0x80        ; Call the kernel
        jmp    read  ;

        

    next_arg: ; move to the next arg and print the current one
        push ebx
        push ecx
        push edx          ; Push the address of the string
        call strlen       ; Call the strlen function
        add esp, 4        ; Adjust the stack pointer
        mov edi, eax
        mov    eax, 4      ; System call number 4 is write
        mov    ebx, 1      ; File descriptor 1 is stdout
        mov    ecx, edx ; Buffer address
        mov    edx, edi       ; Write all the argument 
        int    0x80        ; Call the kernel
        call print_newline
        pop ecx
        pop ebx
        inc    ebx        ; Move to the next argument
        jmp    get_arguments

    exit_program:
        ; Exit program
        mov    eax, 1      ; System call number 1 is exit
        xor    ebx, ebx    ; Exit with return code 0
        int    0x80        ; Call the kernel

    change_z:
        sub dl,25
        jmp print_character

    set_inFile:
        push eax
        push edx
        push ebx
        push ecx
        lea ebp, [edx+2]; 
        mov    eax, 5       ; System call number 5 is open
        mov    ebx, ebp; File name or path
        mov    ecx, 0       ; Flags (0 for read-only)
        int    0x80         ; Call the kernel
        mov [inFile], eax ; copy the file descriptor
        pop ecx
        pop ebx 
        pop edx
        pop eax
        jmp next_arg

    set_outFile:
        push edx
        push ebx
        push ecx
        lea ebp, [edx+2]; 
        mov    eax, 5       ; System call number 5 is open
        mov    ebx, ebp; File name or path
        mov    ecx, 1 | 64 | 512      ; Flags 1 for write only
        mov     edx, 0644 ;
        int    0x80         ; Call the kernel
        mov [outFile], eax ; copy the file descriptor
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
        mov eax, 4       ; System call number 4 is write
        mov ebx, 1       ; File descriptor 1 is stdout
        mov ecx, newline ; Buffer address
        mov edx, 1       ; Write 1 byte
        int 0x80         ; Call the kernel
        ret  

