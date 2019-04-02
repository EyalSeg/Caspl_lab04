section .text
global _start
global system_call
extern main
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

code_start:
global infection
global infect

section .data
msg db  'This file is infected\0',0xa ;our dear string
len equ $ - msg         ;length of our dear string

section .text

infection:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, 4          ; write        
    mov     ebx, 1          ; stdout
    mov     ecx, msg         
    mov     edx, len        
    int     0x80            ; Transfer control to operating system

    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

infect:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    ; OPEN FILE
    mov     eax, 5          ; open        
    mov     ebx, [ebp+8]    ; File name, first arg  
    mov     ecx, 02001Q      ; open for writing, appending at the file's end
    mov     edx, 0
    int     0x80            ; Transfer control to operating system

    mov     ebx, eax    ; Save returned value, file descriptor

    ; WRITE TO FILE
    mov     eax, 4          ; write        
    ; ebx holds file desc from last call
    mov     ecx, code_start ; start writing at the label's address
    mov     edx, code_end - code_start
    ;mov ecx, msg
    ;mov edx, len
    int     0x80            ; Transfer control to operating system

    ; CLOSE FILE
    mov     eax, 6          ; close        
                            ; ebx still holds the file descriptor
    mov     ecx, 0
    mov     edx, 0
    int     0x80            ; Transfer control to operating system


    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

code_end: