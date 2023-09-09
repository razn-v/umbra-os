#include <kernel/syscalls/handler.hpp>
#include <kernel/terminal.hpp>

namespace Syscall {

using SyscallHandler = uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

#define SYSCALL(code, handler) (SyscallHandler)handler,
static SyscallHandler handlers[] = {
    SYSCALLS
};
#undef SYSCALL

void handler(Interrupt::Registers* regs) {
    if (regs->rdi >= SyscallCode::END) {
        Terminal::printf("{red}Invalid syscall!\n");
        return;
    }

    regs->rax = handlers[regs->rdi](regs->rsi, regs->rdx, regs->rcx, regs->r8, regs->r9, regs->r10);
}

}
