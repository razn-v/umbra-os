#pragma once

#include <stdint.h>
#include <stddef.h>
#include <kernel/memory/heap.hpp>
#include <kernel/lock.hpp>

struct ElfSymbol {
    uintptr_t address;
    char* name;
};

extern const ElfSymbol text_start, text_end;
extern const ElfSymbol rodata_start, rodata_end;
extern const ElfSymbol data_start, data_end;

#define PTE_PRESENT (1ull << 0ull)
#define PTE_WRITABLE (1ull << 1ull)
#define PTE_USER (1ull << 2ull)
#define PTE_NX (1ull << 63ull)
#define PTE_GET_ADDR(VALUE) ((VALUE) & 0x000ffffffffff000)

namespace Vmm {

struct AddressSpace {
    Spinlock lock;
    uint64_t* pml4;
 
    ~AddressSpace();
    AddressSpace() {
        // One page long because each PML entry is composed of 512 8-byte long entries
        this->pml4 = (uint64_t*)PHYS_TO_VIRT((uintptr_t)Pmm::calloc(1));
    }

    void map(uintptr_t virt_addr, uintptr_t phys_addr, uint64_t flags);
    void map_io_range(uintptr_t start_addr, uintptr_t end_addr, uint64_t flags);
    void map_range(uintptr_t virt_addr, uintptr_t phys_addr, size_t pages, uint64_t flags); 
    uint64_t virt_to_phys(uint64_t virt_addr);
    void delete_space();
};

void init();
AddressSpace* new_space();
void switch_space(AddressSpace* space);

}
