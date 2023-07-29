#include <stdint.h>

namespace Msr {

enum class MsrType {
    // APIC Base Address Register
    ApicBase = 0x1b,
    // APIC Spurious Interrupt Vector Register
    ApicSivr = 0x80f,
    // APIC End Of Interrupt register
    ApicEOI = 0x80b
};

uint64_t read_msr(MsrType msr);
void write_msr(MsrType msr, uint64_t value);

}
