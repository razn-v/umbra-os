#include <kernel/syscalls/exit.hpp>
#include <kernel/scheduler.hpp>

void sys_exit([[gnu::unused]] int status) {
    Scheduler::kill_and_yield();
}
