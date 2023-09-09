#include <kernel/syscalls/clock.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/terminal.hpp>
#include <kernel/timer.hpp>
#include <errno.h>

void sys_sleep(uint64_t ms) {
    Scheduler::sleep(ms);
}

int sys_clock_get(int clock, time_t *secs, long *nanos) {
    switch (clock) {
        case CLOCK_REALTIME:
        case CLOCK_REALTIME_COARSE: {
            uint64_t ms = Timer::get_real_time();
            *secs = ms / 1000;
            *nanos = ms * 1000000;
            break;
        }
        case CLOCK_BOOTTIME:
        case CLOCK_MONOTONIC:
        case CLOCK_MONOTONIC_RAW:
        case CLOCK_MONOTONIC_COARSE: {
            Terminal::printf("{red}(clock) Not implemented.");
            break;
        }
        case CLOCK_PROCESS_CPUTIME_ID:
        case CLOCK_THREAD_CPUTIME_ID: {
            *secs = 0;
            *nanos = 0;
            break;
        }
        default:
            return -ENOSYS;
    }

    return 0;
}
