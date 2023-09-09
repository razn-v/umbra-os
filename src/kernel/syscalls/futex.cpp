#include <kernel/syscalls/futex.hpp>
#include <kernel/scheduler.hpp>
#include <stdint.h>
#include <errno.h>

// TODO: Check if `pointer` is a valid pointer in userspace
// TODO: Should we care about `time`?
int sys_futex_wait(int* pointer, int expected, [[gnu::unused]] const struct timespec* time) {
    if (*pointer != expected) {
        return -EAGAIN;
    }

    auto task = Scheduler::get_current_task();
    Scheduler::add_futex_handle(task);

    // FIXME: Do we need a lock for `task->futexes`?
    task->futexes.push((uintptr_t)pointer);
    // Block undefinitely, waiting for a futex wake
    Scheduler::block_on([]() { return false; }); 
    task->futexes.remove((uintptr_t)pointer);

    if (task->futexes.is_empty()) {
        Scheduler::remove_futex_handle(task);
    }

    return 0;
}

int sys_futex_wake(int* pointer) {
    Scheduler::wake_futex_handles((uintptr_t)pointer);
    return 0;
}
