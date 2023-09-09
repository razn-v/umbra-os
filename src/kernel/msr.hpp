#include <stdint.h>

namespace Msr {

enum class MsrType : uint64_t {
    // APIC Base Address Register
    ApicBase = 0x1b,
    // APIC Spurious Interrupt Vector Register
    ApicSivr = 0x80f,
    // APIC End Of Interrupt register
    ApicEOI = 0x80b,
    Tcb = 0xc0000100
};

uint64_t read_msr(MsrType msr);
void write_msr(MsrType msr, uint64_t value);

}
