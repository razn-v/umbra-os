#include <kernel/syscalls/sleep.hpp>
#include <kernel/scheduler.hpp>

void sleep(uint64_t ms) {
    Scheduler::sleep(ms);
}
