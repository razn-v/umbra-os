#include <kernel/memory/heap.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/terminal.hpp>
#include <kernel/lock.hpp>

namespace Heap {

static Spinlock slab_lock;

template <size_t slab_size>
Slab<slab_size>* new_slab() {
    auto slab = (Slab<slab_size>*)PHYS_TO_VIRT((uintptr_t)Pmm::calloc(1));
    slab->header.slab_size = slab_size;
    slab->header.free_block = slab->blocks;
    slab->header.next = nullptr;

    // Link each block to the next one
    size_t i;
    for (i = 0; i < slab->MAX_BLOCKS - 1; i++) {
        slab->blocks[i].next = &slab->blocks[i + 1];
    }
    slab->blocks[i].next = nullptr;

    return slab;
}

template <size_t slab_size>
void* Slab<slab_size>::alloc(size_t size) {
    slab_lock.acquire();

    // If the slab is full, we create a new slab (unless it already exists) and make the allocation
    // there
    if (this->header.free_block == nullptr) {
        if (this->header.next == nullptr) this->header.next = new_slab<slab_size>(); 
        auto slab = (Slab<slab_size>*)this->header.next;
        slab_lock.release();

        return slab->alloc(size);
    }

    SlabBlock<slab_size>* block = (SlabBlock<slab_size>*)this->header.free_block;
    this->header.free_block = block->next;

    slab_lock.release();
    return (void*)block;
}

template <size_t slab_size>
void Slab<slab_size>::free(void* addr) {
    slab_lock.acquire();

    SlabBlock<slab_size>* block = (SlabBlock<slab_size>*)addr;
    block->next = this->header.free_block;
    this->header.free_block = block;

    slab_lock.release();
}

static Slab<8>* slab_8b;
static Slab<16>* slab_16b;
static Slab<32>* slab_32b;
static Slab<64>* slab_64b;
static Slab<128>* slab_128b;
static Slab<256>* slab_256b;
static Slab<512>* slab_512b;

void init() {
    slab_8b = new_slab<8>();
    slab_16b = new_slab<16>();
    slab_32b = new_slab<32>();
    slab_64b = new_slab<64>();
    slab_128b = new_slab<128>();
    slab_256b = new_slab<256>();
    slab_512b = new_slab<512>();
}

[[gnu::malloc]] void* kmalloc(size_t size) {
    if (size <= 8) {
        return slab_8b->alloc(size); 
    } else if (size <= 16) {
        return slab_16b->alloc(size);
    } else if (size <= 32) {
        return slab_32b->alloc(size);
    } else if (size <= 64) {
        return slab_64b->alloc(size);
    } else if (size <= 128) {
        return slab_128b->alloc(size);
    } else if (size <= 256) {
        return slab_256b->alloc(size);
    } else if (size <= 512) {
        return slab_512b->alloc(size);
    } else {
        // Allocate directly from the PMM
        size_t pages = ALIGN_UP(size, PAGE_SIZE) / PAGE_SIZE + 1;
        uintptr_t addr = PHYS_TO_VIRT((uintptr_t)Pmm::alloc(pages));

        // Put the header in the first page
        SlabHeader* header = (SlabHeader*)addr;
        header->slab_size = pages;
        header->free_block = nullptr;
        header->next = nullptr;

        // Return the address one page after the header
        return (void*)(addr + PAGE_SIZE);
    }

    return nullptr;
}

void* kcalloc(size_t size) {
    void* addr = Heap::kmalloc(size);
    memset(addr, 0, size);
    return addr;
}

void kfree(void* addr) {
    // Check if the address is aligned to a page boundary. If that's the case, this means that the 
    // address was allocated using the PMM.
    if (((uintptr_t)addr & 0xfff) == 0) {
        SlabHeader* header = (SlabHeader*)((uintptr_t)addr - PAGE_SIZE); 
        Pmm::free((void*)VIRT_TO_PHYS((uintptr_t)header), header->slab_size);
        return;
    }

    uintptr_t page = ALIGN_DOWN((uintptr_t)addr, PAGE_SIZE);

    // Go throught each slab of the same size to search for the slab containing the address we want
    // to free
    SlabHeader* header = (SlabHeader*)page;
    switch (header->slab_size) {
        case 8: {
            Slab<8>* slab = (Slab<8>*)page;
            slab->free(addr);
            break;
        }
        case 16: {
            Slab<16>* slab = (Slab<16>*)page;
            slab->free(addr);
            break;
        }
        case 32: {
            Slab<32>* slab = (Slab<32>*)page;
            slab->free(addr);
            break;
        }
        case 64: {
            Slab<64>* slab = (Slab<64>*)page;
            slab->free(addr);
            break;
        }
        case 128: {
            Slab<128>* slab = (Slab<128>*)page;
            slab->free(addr);
            break;
        }
        case 256: {
            Slab<256>* slab = (Slab<256>*)page;
            slab->free(addr);
            break;
        }
        case 512: {
            Slab<512>* slab = (Slab<512>*)page;
            slab->free(addr);
            break;
        }
        default: {
            Terminal::printf("{red}Tried to free invalid size %d!\n", header->slab_size);
            break;
        }
    }
}

}

void* operator new(size_t size) { return Heap::kmalloc(size); }
void* operator new[](size_t size) { return Heap::kmalloc(size); }
void operator delete(void* addr) { Heap::kfree(addr); }
void operator delete[](void* addr) { Heap::kfree(addr); }
