global gdt_load
gdt_load:
    lgdt [rdi]

    ; Reload data segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; A far return is required for changing CS
    push 0x8
    lea rax, [rel .reload_cs]
    push rax
    retfq

.reload_cs:
    ret

global tss_load
tss_load:
    ltr di
    ret
