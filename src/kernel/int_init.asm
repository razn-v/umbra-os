section .text

extern int_dispatch

int_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    call int_dispatch
    mov rsp, rax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Remove the vector number and the error code
    add rsp, 16

    iretq

%macro INT_ERR 1
int_handler_%1:
    push qword %1
    jmp int_stub
%endmacro

%macro INT_NO_ERR 1
int_handler_%1:
    push qword 0
    push qword %1
    jmp int_stub
%endmacro

INT_NO_ERR 0
INT_NO_ERR 1
INT_NO_ERR 2
INT_NO_ERR 3
INT_NO_ERR 4
INT_NO_ERR 5
INT_NO_ERR 6
INT_NO_ERR 7
INT_ERR 8
INT_NO_ERR 9
INT_ERR 10
INT_ERR 11
INT_ERR 12
INT_ERR 13
INT_ERR 14
INT_NO_ERR 15 ; Reserved
INT_NO_ERR 16
INT_ERR 17
INT_NO_ERR 18
INT_NO_ERR 19
INT_NO_ERR 20 ; Reserved
INT_ERR 21
INT_NO_ERR 22 ; Reserved
INT_NO_ERR 23 ; Reserved
INT_NO_ERR 24 ; Reserved
INT_NO_ERR 25 ; Reserved
INT_NO_ERR 26 ; Reserved
INT_NO_ERR 27 ; Reserved
INT_NO_ERR 28
INT_ERR 29
INT_ERR 30
INT_NO_ERR 31 ; Reserved

; User-defined interrupts
%assign i 32
; Loop from 32 to 255 (included)
%rep 224
    INT_NO_ERR i
%assign i i+1
%endrep

; Save the address of each interrupt
section .data
global int_handlers

int_handlers:
%assign i 0
%rep 256
    dq int_handler_ %+ i
%assign i i+1
%endrep
