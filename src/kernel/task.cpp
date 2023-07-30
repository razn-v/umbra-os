#include <kernel/task.hpp>
#include <kernel/memory/vmm.hpp>
#include <kernel/memory/heap.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/libc/string.hpp>
#include <kernel/gdt.hpp>

// Always incremented
static size_t next_pid = 0;

Task* Task::create(const char* name, void (*entry)(), bool user_mode, Vmm::AddressSpace* page_map) {
    Task* task = new Task;
    task->pid = next_pid++;
    task->status = Task::Status::Ready;
    task->address_space = page_map == nullptr ? Vmm::new_space() : page_map;

    task->context = new Interrupt::Registers;
    memset(task->context, 0, sizeof(Interrupt::Registers));
    task->context->iret_rip = (uint64_t)entry;

    if (user_mode) {
        uintptr_t phys_stack = (uintptr_t)Pmm::alloc(STACK_SIZE / PAGE_SIZE);
        task->address_space->map_range(USER_STACK_BASE - STACK_SIZE, phys_stack, 
                STACK_SIZE / PAGE_SIZE, PTE_PRESENT | PTE_WRITABLE | PTE_NX | PTE_USER);
        task->context->iret_rsp = USER_STACK_BASE;

        task->context->iret_cs = USER_CS | 3;
        task->context->iret_ss = USER_DS | 3;
    } else {
        task->context->iret_rsp = PHYS_TO_VIRT(
            (uintptr_t)Pmm::alloc(STACK_SIZE / PAGE_SIZE) + STACK_SIZE
        );
        task->context->iret_cs = KERNEL_CS;
        task->context->iret_ss = KERNEL_DS;
    }
    // The value 0x202 will clear all flags except for bits 2 and 9. 
    // Bit 2 is a legacy feature and the manual recommends that it's set, and bit 9 is the 
    // interrupts flag.
    task->context->iret_flags = 0x202;

    strcpy(task->name, name);

    return task;
}

Task::~Task() {
    // Free the stack
    Pmm::free((void*)this->address_space->virt_to_phys(USER_STACK_BASE - STACK_SIZE), 
            STACK_SIZE / PAGE_SIZE);

    delete this->address_space;
    delete this->context;
}
