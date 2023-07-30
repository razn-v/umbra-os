#include <kernel/memory/vmm.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/terminal.hpp>
#include <kernel/libc/string.hpp>

namespace Vmm {

static AddressSpace* kernel_space;

static uint64_t* get_next_level(uint64_t* top_level, size_t idx, bool allocate = false) {
    if ((top_level[idx] & PTE_PRESENT) != 0) {
        return (uint64_t*)PHYS_TO_VIRT(PTE_GET_ADDR(top_level[idx]));
    }

    if (!allocate) {
        return nullptr;
    }

    uint64_t* next_level = (uint64_t*)Pmm::calloc(1);
    if (next_level == nullptr) {
        return nullptr;
    }

    top_level[idx] = (uint64_t)next_level | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    return (uint64_t*)(PHYS_TO_VIRT((uintptr_t)next_level));
}

// We don't have any identity mapping, that way implementing user-space will be easier. This also
// means that all of our addresses in our functions need to be virtual, as physical addresses will 
// only be supported when switching from limine to our own mappings. 
// All mappings are supervisor-only by default.
void init() {
    kernel_space = new AddressSpace;

    // Since the higher half has to be shared amongst all address spaces, we need to initialise
    // every single higher half PML3 so they can be shared. If we don't, then when a new higher half
    // PML3 will be created, it will only exist for the process that created it.
    for (size_t i = 256; i < 512; i++) {
        get_next_level(kernel_space->pml4, i, true);
    } 

    // Those are all virtual addresses of the different sections of the kernel
    uintptr_t text_start_addr = ALIGN_DOWN((uintptr_t)&text_start, PAGE_SIZE);
    uintptr_t text_end_addr = ALIGN_UP((uintptr_t)&text_end, PAGE_SIZE);
    uintptr_t rodata_start_addr = ALIGN_DOWN((uintptr_t)&rodata_start, PAGE_SIZE);
    uintptr_t rodata_end_addr = ALIGN_UP((uintptr_t)&rodata_end, PAGE_SIZE);

    uintptr_t data_start_addr = ALIGN_DOWN((uintptr_t)&data_start, PAGE_SIZE);
    uintptr_t data_end_addr = ALIGN_UP((uintptr_t)&data_end, PAGE_SIZE);

    // We map the different sections of the kernel, as done by limine itself. This ensure that our
    // sections have the correct permissions, and can be useful when debugging. Alternatively, we
    // could map the entire kernel with the same permissions, as done by some kernels.
    kernel_space->map_io_range(text_start_addr, text_end_addr, PTE_PRESENT);
    kernel_space->map_io_range(rodata_start_addr, rodata_end_addr, PTE_PRESENT | PTE_NX);
    kernel_space->map_io_range(data_start_addr, data_end_addr, PTE_PRESENT | PTE_WRITABLE | PTE_NX);

    // Map the first 4gb (excluding the first 4k) to ensure most devices are mapped
    for (uintptr_t addr = 0x1000; addr < 0x100000000; addr += PAGE_SIZE) {
        kernel_space->map(PHYS_TO_VIRT(addr), addr, PTE_PRESENT | PTE_WRITABLE | PTE_NX);
    }

    struct limine_memmap_response* memmap = memmap_request.response;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];

        uintptr_t base = ALIGN_DOWN(entry->base, PAGE_SIZE);
        uintptr_t top = ALIGN_UP(entry->base + entry->length, PAGE_SIZE);
        // Skip already mapped areas
        if (top <= 0x100000000) {
            continue;
        }

        for (uintptr_t j = base; j < top; j += PAGE_SIZE) {
            // Skip already mapped areas
            if (j < 0x100000000) {
                continue;
            }
            kernel_space->map(PHYS_TO_VIRT(j), j, PTE_PRESENT | PTE_WRITABLE | PTE_NX);
        }
    }
 
    Vmm::switch_space(kernel_space);
}

AddressSpace* new_space() {
    AddressSpace* space = new AddressSpace;
    
    // Map the kernel to the new address space
    for (size_t i = 256; i < 512; i++) {
        space->pml4[i] = kernel_space->pml4[i];
    }

    return space;
}

void switch_space(AddressSpace* space) {
    asm volatile("mov %0, %%cr3" :: "r"(VIRT_TO_PHYS((uintptr_t)space->pml4)) : "memory");
}

uint64_t* virt_to_pte(uint64_t* pml4, uint64_t virt_addr) {
    uint64_t pml4_offset = (virt_addr >> 39) & 0x1ff;
    uint64_t pml3_offset = (virt_addr >> 30) & 0x1ff;
    uint64_t pml2_offset = (virt_addr >> 21) & 0x1ff;
    uint64_t pml1_offset = (virt_addr >> 12) & 0x1ff;

    uint64_t* pml3 = get_next_level(pml4, pml4_offset, true);
    if (pml3 == nullptr) {
        return nullptr; 
    }

    uint64_t* pml2 = get_next_level(pml3, pml3_offset, true);
    if (pml2 == nullptr) {
        return nullptr;
    }

    uint64_t* pml1 = get_next_level(pml2, pml2_offset, true);
    if (pml1 == nullptr) {
        return nullptr;
    }

    return &pml1[pml1_offset];
}

void AddressSpace::map(uintptr_t virt_addr, uintptr_t phys_addr, uint64_t flags) {
    this->lock.acquire();

    uint64_t* pte = Vmm::virt_to_pte(this->pml4, virt_addr);
    if (pte == nullptr) {
        this->lock.release();
        return;
    }

    // Check if the page table entry is already mapped
    if ((*pte & PTE_PRESENT) != 0) {
        this->lock.release();
        return;
    }

    *pte = phys_addr | flags;
    this->lock.release();
}

void AddressSpace::map_io_range(uintptr_t start_addr, uintptr_t end_addr, uint64_t flags) {
    for (uintptr_t addr = start_addr; addr < end_addr; addr += PAGE_SIZE) {
        uintptr_t phys = IO_TO_PHYS(addr);
        this->map(addr, phys, flags);
    }
}

void AddressSpace::map_range(uintptr_t virt_addr, uintptr_t phys_addr, size_t pages, 
        uint64_t flags) {
    for (size_t i = 0; i < pages * PAGE_SIZE; i += PAGE_SIZE) {
        this->map(virt_addr + i, phys_addr + i, flags);
    }
}

uint64_t AddressSpace::virt_to_phys(uint64_t virt_addr) {
    uint64_t* pte = Vmm::virt_to_pte(this->pml4, virt_addr);
    if (pte == nullptr || (*pte & PTE_PRESENT) == 0) {
        return UINT64_MAX;
    }

    return PTE_GET_ADDR(*pte);
}

void destroy_pml(uint64_t* pml, size_t start, size_t end, size_t level) {
    if (level == 0) {
        return;
    }

    for (size_t i = start; i < end; i++) {
        uint64_t* next_pml = get_next_level(pml, i);
        if (next_pml == nullptr) {
            continue;
        }

        // Destroy all entries of the PML
        destroy_pml(pml, 0, 512, level - 1);
    }

    // A PML is always 1 page long
    Pmm::free((void*)VIRT_TO_PHYS((uintptr_t)pml), 1);
}

AddressSpace::~AddressSpace() {
    this->lock.acquire();

    // Destroy the lower half of the mapping, because the higher half belongs to the kernel
    Vmm::destroy_pml(this->pml4, 0, 256, 4);
    delete this;
}

}
