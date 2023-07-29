#pragma once

#include <stddef.h>
#include <kernel/memory/pmm.hpp>
#include <kernel/libc/misc.hpp>

namespace Heap {

template <size_t slab_size>
struct SlabBlock {
    // Pointer to the next free block
    void* next;
    // A padding is necessary because we want free blocks to take as much space as a used block
    char padding[slab_size - sizeof(void*)];
};

template <size_t slab_size> struct Slab;

// Header at the start of each page of a slab
struct SlabHeader {
    size_t slab_size;
    // Pointer to the first free block of the slab
    void* free_block;
    // Pointer to the next slab, allocated only whent the current slab is full
    void* next;
};

template <size_t slab_size>
struct Slab {
    static constexpr size_t HEADER_SIZE = sizeof(SlabHeader);
    static constexpr size_t MAX_BLOCKS = (PAGE_SIZE - HEADER_SIZE) / sizeof(SlabBlock<slab_size>);

    SlabHeader header;
    // A padding is necessary so that the header takes exactly one block of the slab.
    // There is probably a less wasteful way to do this.
    char padding[slab_size > HEADER_SIZE ? slab_size - HEADER_SIZE : 0];
    SlabBlock<slab_size> blocks[MAX_BLOCKS];

    void* alloc(size_t size);
    void free(void* addr);
};

template <size_t slab_size>
Slab<slab_size>* new_slab();

void init();

void* kmalloc(size_t size);
void* kcalloc(size_t size);
void kfree(void* addr);

}

void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* addr);
void operator delete[](void* addr);
