#include <kernel/syscalls/log.hpp>
#include <kernel/terminal.hpp>

uint64_t sys_log(const char* msg) {
    Terminal::printf("{green}%s\n", msg);
    return 0;
}
