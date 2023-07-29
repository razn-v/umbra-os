#pragma once

#include <stdint.h>

#define GDT_ENTRIES 7

#define KERNEL_CS  0x08
#define KERNEL_DS  0x10
#define USER_CS    0x18
#define USER_DS    0x20
#define TSS_OFFSET 0x28

// A generic segment descriptor
struct [[gnu::packed]] GdtEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    //uint8_t type : 4 = 0;
    //uint8_t user_seg : 1 = 1;
    //uint8_t dpl : 2 = 0;
    //uint8_t present : 1 = 1;
    uint8_t flags;
    uint8_t limit_high : 4;
    //uint8_t : 1;
    //uint8_t long_mode : 1 = 1;
    //uint8_t op_size : 1 = 0;
    //uint8_t granularity : 1 = 0;
    uint8_t granularity : 4;
    uint8_t base_high;

    constexpr GdtEntry() = default;

    constexpr GdtEntry(uint32_t base, uint32_t limit, uint8_t flags, uint8_t granularity) : 
        limit_low(limit & 0xffff),
        base_low(base & 0xffff),
        base_mid((base >> 16) & 0xff),
        flags(flags),
        limit_high((limit >> 16) & 0xf),
        granularity(granularity),
        base_high((base >> 24) & 0xff) {}
};

// System descriptor
struct [[gnu::packed]] GdtSysEntry {
    uint16_t limit_low;
    uint16_t base0;
    uint8_t base1;
    //uint8_t type : 4;
    //uint8_t : 1 = 0;
    //uint8_t dpl : 2;
    //uint8_t present : 1 = 1;
    uint8_t flags;
    uint8_t limit_high : 4;
    //uint8_t avl : 1;
    //uint8_t : 2;
    //uint8_t granularity : 1;
    uint8_t granularity : 4;
    uint8_t base2;
    uint32_t base3;
    uint8_t : 8;
    uint8_t zero: 5 = 0;
    uint32_t : 19;

    constexpr GdtSysEntry() = default;

    constexpr GdtSysEntry(uint64_t base, uint32_t limit, uint8_t flags, uint8_t granularity) : 
        limit_low(limit & 0xffff),
        base0(base & 0xffff),
        base1((base >> 16) & 0xff),
        flags(flags),
        limit_high((limit >> 16) & 0xf),
        granularity(granularity),
        base2((base >> 24) & 0xff),
        base3((base >> 32) & 0xffffffff) {}
};

extern "C" void gdt_load(void const* gdt);
extern "C" void tss_load(uint16_t gdt_offset);

struct [[gnu::packed]] GdtDesc {
    uint16_t limit;
    uintptr_t address;

    void load() const {
        gdt_load(this);
    }
};

static GdtEntry gdt_entries[GDT_ENTRIES] = {
    // Null segment
    GdtEntry(0, 0, 0, 0),
    // Kernel code segment
    GdtEntry(0, 0, 0b10011011, 0b0010),
    // Kernel data segment
    GdtEntry(0, 0, 0b10010011, 0b0010),
    // User code segment
    GdtEntry(0, 0, 0b11111011, 0b0010),
    // User data segment
    GdtEntry(0, 0, 0b11110011, 0b0010),
    // TSS upper and lower 8 bytes, later filled
    GdtEntry(0, 0, 0, 0),
    GdtEntry(0, 0, 0, 0),
};

static GdtDesc gdt = {
    .limit = GDT_ENTRIES * sizeof(GdtEntry) - 1,
    .address = (uintptr_t)gdt_entries
};

struct [[gnu::packed]] Tss {
    uint32_t : 32;
    uint64_t rsp[3];
    uint64_t : 64;
    uint64_t ist[7];
    uint64_t : 64;
    uint16_t : 16;
    uint16_t io_bitmap_offset;
};

void init_tss(Tss* tss);
