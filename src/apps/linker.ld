OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

SECTIONS
{
    . = 4K;

    text : ALIGN(4K) {
        *(.text .text.*)
    }

    rodata : ALIGN(4K) {
        init_array_start = .;
        *(.init_array)
        init_array_end = .;

        fini_array_start = .;
        *(.fini_array)
        fini_array_end = .;

        *(.rodata .rodata.*)
    }

    data : ALIGN(4K) {
        *(.data .data.*)
    }

    bss : ALIGN(4K) {
        *(COMMON)
        *(.bss .bss.*)
    }

    /DISCARD/ : {
        *(.eh_frame)
    }
}
