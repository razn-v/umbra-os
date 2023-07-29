#include <stdint.h>
#include <kernel/syscalls/exit.hpp>
#include <kernel/syscalls/sleep.hpp>
#include <kernel/syscalls/test.hpp>

#define SYSCALLS \
    SYSCALL(Exit, exit) \
    SYSCALL(Sleep, sleep) \
    SYSCALL(Test, test) \

#define SYSCALL(code, handler) code, 
enum SyscallCode {
    SYSCALLS
    END
};
#undef SYSCALL

inline uint64_t syscall(uint64_t code, uint64_t arg1 = 0, uint64_t arg2 = 0, uint64_t arg3 = 0, 
        uint64_t arg4 = 0, uint64_t arg5 = 0) {
    uint64_t ret;
    asm volatile(
        "movq %[code], %%rdi\n"
        "movq %[arg1], %%rsi\n"
        "movq %[arg2], %%rdx\n"
        "movq %[arg3], %%rcx\n"
        "movq %[arg4], %%r8\n"
        "movq %[arg5], %%r9\n"
        "int $0xfe\n"
        "movq %%rax, %[ret]"
        : [ret] "=r"(ret)
        : [code] "r"((uint64_t)code), [arg1] "r"(arg1), [arg2] "r"(arg2), [arg3] "r"(arg3), [arg4] "r"(arg4), [arg5] "r"(arg5)
        : "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9", "%rax"
    );
    return ret;
}
