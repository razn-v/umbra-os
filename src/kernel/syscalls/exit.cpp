#include <kernel/syscalls/exit.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/terminal.hpp>

[[gnu::noreturn]] void exit(int exit_code) {
    Terminal::printf("received exit code %d\n", exit_code);
    Scheduler::kill_and_yield();
    __builtin_unreachable();
}
