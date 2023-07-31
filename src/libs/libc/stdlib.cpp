#include "stdlib.hpp"
#include <syscalls.hpp>

[[gnu::noreturn]] void exit(int status) {
    syscall(SyscallCode::Exit, status);
    __builtin_unreachable();
}

void sleep(uint64_t ms) {
    syscall(SyscallCode::Sleep, ms);
}

KeyboardEvent poll_event() {
    KeyboardEvent event;
    syscall(SyscallCode::PollEvent, (uint64_t)(&event));
    return event;
}

// Temporary syscall, will be deleted afterwards
void putchar(char ch) {
    syscall(SyscallCode::Putchar, ch);
}
