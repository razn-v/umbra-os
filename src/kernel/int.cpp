#include <kernel/int.hpp>
#include <kernel/terminal.hpp>
#include <kernel/io.hpp>
#include <kernel/apic.hpp>
#include <kernel/drivers/keyboard.hpp>
#include <kernel/timer.hpp>
#include <kernel/syscalls/handler.hpp>

namespace Interrupt {

static irq_handler handlers[256] = { NULL };

void init() {
    Interrupt::set_handler(TIMER_VECT, Timer::Pit::handler);
    Interrupt::set_handler(KEYBOARD_VECT, Keyboard::handler);
    Interrupt::set_handler(SYSCALL_VECT, Syscall::handler);
}

extern "C" uintptr_t int_dispatch(Registers* regs) {
    auto handler = handlers[regs->vect_num];
    if (handler) {
        handler(regs);
    } else {
        Terminal::printf("Received interrupt %d!\n", regs->vect_num);
        asm volatile("hlt");
    }

    Apic::Local::write_reg(Apic::Local::Register::Eoi, 0);
    return (uintptr_t)regs;
}

void set_handler(uint64_t vect, irq_handler handler) {
    handlers[vect] = handler;
}

bool interrupts_enabled() {
    uint64_t flags;
    asm volatile("pushf; pop %0" : "=rm"(flags));
    return flags & (1 << 9);
}

}
