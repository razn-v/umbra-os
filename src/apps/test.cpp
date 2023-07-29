#include <kernel/syscalls/syscalls.hpp>

int main() {
    syscall(SyscallCode::Test);
    syscall(SyscallCode::Sleep, 5 * 1000);
    syscall(SyscallCode::Test);
    return 0;
}
