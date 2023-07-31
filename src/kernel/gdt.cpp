#include <kernel/gdt.hpp>
#include <kernel/memory/pmm.hpp>

namespace Gdt {

static GdtDesc gdt = {
    .limit = GDT_ENTRIES * sizeof(GdtEntry) - 1,
    .address = (uintptr_t)gdt_entries
};

static Tss tss;

void load() {
    gdt.load();
}

void init_tss() {
    memset(&tss, 0, sizeof(Gdt::Tss)); 
    tss.rsp[0] = PHYS_TO_VIRT((uintptr_t)Pmm::alloc(STACK_SIZE / PAGE_SIZE) + STACK_SIZE);

    // The limit field is ignored in long mode, but it's best to set it to max value in case we
    // support compatibility mode in the future.
    //
    // 0b10001001 : type=0b1001, dpl=0, present=1
    Gdt::GdtSysEntry tss_entry = Gdt::GdtSysEntry((uint64_t)&tss, 0xffff, 0b10001001, 0);

    // Because the TSS entry is 16 bytes long and each GDT descriptor is 8 byte long, we need to
    // split the entry in two
    auto entry0 = (Gdt::GdtEntry*)&tss_entry; 
    auto entry1 = (Gdt::GdtEntry*)&(tss_entry.base3); 

    Gdt::gdt_entries[5] = *entry0;
    Gdt::gdt_entries[6] = *entry1;

    // Reload the GDT and load the TSS
    Gdt::load();
    Gdt::tss_load(TSS_OFFSET);
}

}
