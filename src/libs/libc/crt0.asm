section .text

extern pre_main

global _start:
_start:
    call pre_main
