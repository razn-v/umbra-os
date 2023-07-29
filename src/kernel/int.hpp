#pragma once

#include <stdint.h>

namespace Interrupt {

#define TIMER_VECT    32 
#define KEYBOARD_VECT 33
#define SYSCALL_VECT  0xfe

struct Registers {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t vect_num;
    uint64_t err_code;

    uint64_t iret_rip;
    uint64_t iret_cs;
    uint64_t iret_flags;
    uint64_t iret_rsp;
    uint64_t iret_ss;
};

void init();

extern "C" uintptr_t int_dispatch(Registers *regs);

typedef void (*irq_handler)(Registers *regs);
void set_handler(uint64_t vect, irq_handler handler);

inline void enable_interrupts() {
    asm volatile("sti");
}

inline void disable_interrupts() {
    asm volatile("cli");
}

bool interrupts_enabled();

}
