#include <kernel/syscalls/test.hpp>
#include <kernel/terminal.hpp>

uint64_t test() {
    Terminal::printf("test\n");
    return 0;
}
