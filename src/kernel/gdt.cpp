#include <kernel/gdt.hpp>

void init_tss(Tss* tss) {
    // The limit field is ignored in long mode, but it's best to set it to max value in case we
    // support compatibility mode in the future.
    //
    // 0b10001001 : type=0b1001, dpl=0, present=1
    GdtSysEntry tss_entry = GdtSysEntry((uint64_t)tss, 0xffff, 0b10001001, 0);

    // Because the TSS entry is 16 bytes long and each GDT descriptor is 8 byte long, we need to
    // split the entry in two
    GdtEntry* entry0 = (GdtEntry*)&tss_entry; 
    GdtEntry* entry1 = (GdtEntry*)&(tss_entry.base3); 

    gdt_entries[5] = *entry0;
    gdt_entries[6] = *entry1;

    // Reload the GDT and load the TSS
    gdt.load();
    tss_load(TSS_OFFSET);
}
