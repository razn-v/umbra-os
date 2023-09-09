#include <kernel/syscalls/mmap.hpp>
#include <kernel/terminal.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/memory/vmm.hpp>
#include <kernel/scheduler.hpp>
#include <sys/mman.h>

void* sys_mmap(void* hint, size_t size, int prot, int flags, int fd, off_t offset) {
    if (fd != -1 || offset != 0 || (flags & MAP_ANONYMOUS) == 0 || (flags & MAP_SHARED)) {
        Terminal::printf("{red}(mmap) Not implemented!\n");
        return nullptr;
    }

    uintptr_t addr = (uintptr_t)Pmm::alloc(size / PAGE_SIZE);
    auto task = Scheduler::get_current_task();

    uintptr_t base = task->heap_cur;
    task->heap_cur += size;

    int page_flags = PTE_PRESENT | PTE_USER;
    if (prot & PROT_WRITE) {
        page_flags |= PTE_WRITABLE;
    }
    if ((prot & PROT_EXEC) == 0) {
        page_flags |= PTE_NX;
    }

    if (hint == 0) {
        task->address_space->map_range(base, addr, size / PAGE_SIZE, page_flags);
    } else {
        task->address_space->map_range((uintptr_t)hint, addr, size / PAGE_SIZE, page_flags);
    }
    
    return (void*)base;
}
