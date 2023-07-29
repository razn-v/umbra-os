#include <kernel/msr.hpp>

namespace Msr {

uint64_t read_msr(MsrType msr) {
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t) high << 32) | low;
}

void write_msr(MsrType msr, uint64_t value) {
    uint32_t low = value & 0xffffffff;
    uint32_t high = value >> 32;
    asm volatile("wrmsr" :: "a"(low), "d"(high), "c"(msr));
}

}
