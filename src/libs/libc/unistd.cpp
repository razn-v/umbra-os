#include <kernel/syscalls/syscalls.hpp>
#include "unistd.hpp"

[[gnu::noreturn]] void _exit(int status) {
    syscall(SyscallCode::Exit, status);
    __builtin_unreachable();
}
