#include <kernel/int.hpp>

#define PIT_FREQUENCY   1193182

#define PIT_CMD         0x43
#define PIT_COUNTER_0   0x40

#define PIT_CHANNEL_0   0b00  << 6
#define PIT_CHANNEL_1   0b01  << 6
#define PIT_CHANNEL_2   0b10  << 6
#define PIT_READ_BACK   0b11  << 6

#define PIT_LATCH_COUNT 0b00  << 4
#define PIT_LO_ACCESS   0b01  << 4
#define PIT_HI_ACCESS   0b10  << 4
#define PIT_LOHI_ACCESS 0b11  << 4

#define PIT_MODE_0      0b000 << 1
#define PIT_MODE_1      0b001 << 1
#define PIT_MODE_2      0b010 << 1
#define PIT_MODE_3      0b011 << 1
#define PIT_MODE_4      0b100 << 1
#define PIT_MODE_5      0b101 << 1

#define PIT_BINARY_MODE 0b0
#define PIT_BCD_MODE    0b1

namespace Timer {

void init();

namespace Pit {
    void handler(Interrupt::Registers* regs);
}

namespace Lapic {
    void handler(Interrupt::Registers *regs);
    void stop();
    void resume();
}

uint64_t ms_to_cycles(uint64_t ms);
uint64_t get_real_time();

}
