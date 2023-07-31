#include <kernel/scheduler.hpp>
#include <kernel/timer.hpp>
#include <kernel/memory/vmm.hpp>
#include <kernel/libc/string.hpp>

// A horrible round-robin scheduler. It might be broken, idk, we'll see.
namespace Scheduler {

static Task* current_task = nullptr;
static Task* idle_task = nullptr;

TaskList queue;

void idle_func() {
    for (;;) asm volatile("hlt");
}

void init() {
    idle_task = Task::create("idle", idle_func);
    current_task = idle_task;
}

void schedule(Interrupt::Registers* context) {
    // Prevent nested schedules
    Timer::Lapic::stop();

    if (queue.is_empty()) {
        // Scheduler not initialized yet
        if (idle_task == nullptr) {
            Timer::Lapic::resume();
            return;
        }
        current_task = idle_task;
    } else if (current_task->status != Task::Status::Ready) {
        // If the task is already running or waiting, save the current context.
        // We don't want the context to be saved in case `current_task` got assigned to `idle_task`
        // because that would put a wrong context in the task.
        memcpy(current_task->context, context, sizeof(Interrupt::Registers));
    }

    // Switch to the next task
    if (!queue.is_empty()) {
        current_task = queue.next_task();
    }

    while (current_task->status != Task::Status::Running) {
        switch (current_task->status) {
            case Task::Status::Ready: {
                current_task->status = Task::Status::Running;
                goto end;
            }
            case Task::Status::Waiting: {
                if (__rdtsc() >= current_task->tsc_sleep) {
                    // The task has finished sleeping
                    current_task->status = Task::Status::Running;
                } else if (queue.next_task() != current_task) {
                    // Go to the next task available
                    current_task = queue.get_current();
                } else {
                    // No task available has been found
                    current_task = idle_task;
                }
                break;
            }
            default:
                break;
        }
    }

end:

    // Load the context and address space of the new task
    memcpy(context, current_task->context, sizeof(Interrupt::Registers));
    Vmm::switch_space(current_task->address_space);

    Timer::Lapic::resume();
}

void yield() {
    Timer::Lapic::stop();
    // FIXME: This is ugly. Sending IPI would be better?
    asm volatile("int %0" : : "i"(TIMER_VECT));
}

void kill_and_yield() {
    if (queue.is_empty()) {
        return;
    }

    delete current_task;
    queue.remove_current();
    Scheduler::yield();
}

void add_task(Task* task) {
    queue.add_task(task);
}

Task* get_current_task() {
    return current_task;
}

void sleep(uint64_t ms) {
    current_task->status = Task::Status::Waiting;
    current_task->tsc_sleep = __rdtsc() + Timer::ms_to_cycles(ms);
    Scheduler::yield();
}

}
