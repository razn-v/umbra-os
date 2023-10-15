#include <kernel/task.hpp>
#include <kernel/memory/vmm.hpp>
#include <kernel/memory/heap.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/libc/string.hpp>
#include <kernel/gdt.hpp>
#include <elf.h>

// Always incremented
static size_t next_pid = 0;

Task* Task::create(const char* name, void (*entry)(), bool user_mode, Vmm::AddressSpace* page_map,
        auxval* auxv) {
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

        // Ring 3
        task->context->iret_cs = USER_CS | 3;
        task->context->iret_ss = USER_DS | 3;

        const char* argv[] = {NULL};
        const char* envp[] = {NULL};

        uint64_t* stack = (uint64_t*)PHYS_TO_VIRT(phys_stack + STACK_SIZE);
        uint64_t* stack_top = stack;

        // Copy envp to the stack
        size_t envp_len;
        for (envp_len = 0; envp[envp_len] != NULL; envp_len++) {
            size_t length = strlen(envp[envp_len]);
            stack = (uint64_t*)((uintptr_t)stack - length - 1);
            memcpy(stack, envp[envp_len], length);
        }

        // Copy argv to the stack
        size_t argv_len;
        for (argv_len = 0; argv[argv_len] != NULL; argv_len++) {
            size_t length = strlen(argv[argv_len]);
            stack = (uint64_t*)((uintptr_t)stack - length - 1);
            memcpy(stack, argv[argv_len], length);
        }

        // Align to 16 bytes
        stack = (uint64_t*)ALIGN_DOWN((uintptr_t)stack, 16);
        if ((argv_len + envp_len) % 2 == 0) {
            stack--;
        }

        #define PUSH_TO_STACK(value) stack--; *stack = value;  
        
        // Push auxilary vector to the stack
        PUSH_TO_STACK(0)
        PUSH_TO_STACK(0)
        PUSH_TO_STACK(0)
        PUSH_TO_STACK(AT_SECURE)
        PUSH_TO_STACK(auxv->at_entry)
        PUSH_TO_STACK(AT_ENTRY)
        PUSH_TO_STACK(auxv->at_phdr)
        PUSH_TO_STACK(AT_PHDR)
        PUSH_TO_STACK(auxv->at_phent)
        PUSH_TO_STACK(AT_PHENT)
        PUSH_TO_STACK(auxv->at_phnum)
        PUSH_TO_STACK(AT_PHNUM)

        uintptr_t old_rsp = task->context->iret_rsp;

        // End of envp
        PUSH_TO_STACK(0)
        // Push envp pointers
        for (size_t i = 0; i < envp_len; i++) {
            old_rsp -= strlen(envp[i]) + 1;
            PUSH_TO_STACK(old_rsp)
        }

        // End of argv
        PUSH_TO_STACK(0)
        // Push argv pointers
        for (size_t i = 0; i < argv_len; i++) {
            old_rsp -= strlen(argv[i]) + 1;
            PUSH_TO_STACK(old_rsp)
        }

        // Push argc
        PUSH_TO_STACK(argv_len)

        task->context->iret_rsp -= (uintptr_t)stack_top - (uintptr_t)stack;
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
    task->add_file_handle(Vfs::get_stdin());
    task->add_file_handle(Vfs::get_stdout());
    task->add_file_handle(Vfs::get_stderr());
    task->events = new Ringbuffer<Keyboard::KeyboardEvent, 5>;

    return task;
}

int Task::add_file_handle(Vfs::FileDescriptor* fd) {
    if (fd == nullptr) {
        return -1;
    }

    if (fd->id == -1) {
        fd->id = this->next_id;
    }

    this->next_id++;
    this->handles.push(fd);
    return fd->id;
}

int Task::add_socket_handle(Socket::Handle* handle) {
    if (handle == nullptr) {
        return -1;
    }

    if (handle->id == -1) {
        handle->id = this->next_id;
    }

    this->next_id++;
    this->socket_handles.push(handle);
    return handle->id;
}

Vfs::FileDescriptor* Task::get_file_handle(int id) {
    auto current = this->handles.head;
    while (current != nullptr) {
        if (current->value->id == id) {
            return current->value;
        }
        current = current->next;
    }
    return nullptr;
}

Socket::Handle* Task::get_socket_handle(int id) {
    auto current = this->socket_handles.head;
    while (current != nullptr) {
        if (current->value->id == id) {
            return current->value;
        }
        current = current->next;
    }
    return nullptr;
}

/*
Socket::Handle* Task::get_socket_handle_by_port(uint16_t port) {
    auto current = this->socket_handles.head;
    while (current != nullptr) {
        if (current->value->port == port) {
            return current->value;
        }
        current = current->next;
    }
    return nullptr;
}
*/

Task::~Task() {
    // Free the stack
    Pmm::free((void*)this->address_space->virt_to_phys(USER_STACK_BASE - STACK_SIZE), 
            STACK_SIZE / PAGE_SIZE);

    delete this->address_space;
    delete this->context;
    delete this->events;
    delete this->working_dir;
}
