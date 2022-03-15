section .rodata
    message: db "Hello, Infected File", 10, 0	; string
    messageSize: equ $ - message	; string_size
section .text
global _start
global infector
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

infection:
    mov edx, messageSize    ; 4th arg - string size
    sub edx, 1
    mov ecx, message        ; 3rd arg - string pointer
    mov eax, 4              ; 1st arg - SYS_WRITE
    int 0x80                ; system_call
    jmp closeFile                     ; Back to caller

infector:                   ;func that void infector(char*)
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state
    sub esp, 4              ; local variable

    openFile:
        mov edx, 0644       ; 4th arg - mode
        mov ecx, 1024       ; 3rd arg - flags O_APPEND
        or ecx, 1           ; or O_WRONLY
        mov ebx, [ebp+8]    ; 2nd arg - pointer to file name
        mov eax, 5          ; 1st arg - SYS_OPEN
        int 0x80            ; system_call
        mov [ebp-4], eax    ; save returned value - fd

    write:           
        mov ebx, [ebp-4]    ; 2nd arg - fd       
        jmp infection
    
    closeFile:
        mov ebx,[ebp-4]     ; 2nd arg - fd
        mov eax, 6          ; 1st arg - SYS_CLOSE
        int 0x80            ; system_call

    add esp, 4              ; restore space of local variable
    popad                   ; Restore caller state (registers)
    pop ebp                 ; Restore caller state
    ret                     ; Back to caller

code_end: