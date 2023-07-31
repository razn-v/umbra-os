#include <kernel/gdt.hpp>

namespace Idt {

struct [[gnu::packed]] IdtEntry {
    uint16_t address_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t address_mid;
    uint32_t address_high;
    uint32_t reserved;

    IdtEntry() = default;

    IdtEntry(uintptr_t handler, uint8_t flags) :
           address_low(handler & 0xffff),
           selector(KERNEL_CS),
           ist(0),
           flags(flags),
           address_mid((handler >> 16) & 0xffff),
           address_high(handler >> 32),
           reserved(0) {}
};

extern "C" void idt_load(void const *idt);
// Addresses of the interrupts handlers
extern "C" uintptr_t int_handlers[256];

struct [[gnu::packed]] IdtDesc {
    uint16_t limit;
    uintptr_t base;

    void load() const {
        idt_load(this);
    }
};

void init();

}
