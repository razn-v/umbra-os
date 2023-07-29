#include <kernel/syscalls/sleep.hpp>
#include <kernel/scheduler.hpp>

uint64_t sleep(uint64_t ms) {
    Scheduler::sleep(ms);
    return 0;
}
