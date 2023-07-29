#include <kernel/syscalls/handler.hpp>
#include <kernel/syscalls/syscalls.hpp>
#include <kernel/terminal.hpp>

namespace Syscall {

using SyscallHandler = uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

#define SYSCALL(code, handler) (SyscallHandler)handler,
static SyscallHandler handlers[] = {
    SYSCALLS
};
#undef SYSCALL

void handler(Interrupt::Registers* regs) {
    if (regs->rdi >= SyscallCode::END) {
        Terminal::printf("{red}invalid syscall!\n");
        return;
    }

    handlers[regs->rdi](regs->rsi, regs->rdx, regs->rcx, regs->r8, regs->r9);

    /*
    if (regs->rdi == 0x1337) {
        Scheduler::kill_and_yield();
        __builtin_unreachable();
    } else if(regs->rdi == 2) {
        Terminal::printf("Machin\n");
    }
    */
}

}
