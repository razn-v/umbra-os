#include <kernel/idt.hpp>
#include <kernel/int.hpp>

namespace Idt {

static IdtEntry idt_entries[256];

static IdtDesc idt = {
    .limit = 0xfff,
    .base = (uintptr_t)idt_entries
};

void init() {
    // Setup each of entry of the IDT
    for (int i = 0; i < 256; i++) {
        if (i == SYSCALL_VECT) {
            // Type: 0b1110 (interrupt gate)
            // Reserved: 0b0
            // DPL: 0b11
            // Present: 0b1
            idt_entries[i] = IdtEntry(int_handlers[i], 0b11101110);
        } else {
            // Type: 0b1110 (interrupt gate)
            // Reserved: 0b0
            // DPL: 0b00
            // Present: 0b1
            idt_entries[i] = IdtEntry(int_handlers[i], 0b10001110);
        }
    } 

    idt.load();
}

}
