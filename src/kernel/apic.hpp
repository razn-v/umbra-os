#include <stdint.h>

namespace Apic {

#define PIC_COMMAND_MASTER 0x20
#define PIC_COMMAND_SLAVE  0xA0
#define PIC_DATA_MASTER    0x21
#define PIC_DATA_SLAVE     0xA1

#define ICW_1              0x11
#define ICW_2_M            0x20
#define ICW_2_S            0x28
#define ICW_3_M            0x02
#define ICW_3_S            0x04
#define ICW_4              0x01

void init();
void disable_pic();

namespace Local {
    enum class Register {
        // Spurious Interrupt Vector
        Siv = 0xf0,
        // End of interrupt
        Eoi = 0xb0,
        // Timer entry in the LVT
        TimerLvt = 0x320,
        TimerInitCount = 0x380,
        TimerCount = 0x390,
        TimerDivisor = 0x3e0
    };

    void init();

    uint32_t read_reg(Local::Register reg);
    void write_reg(Local::Register reg, uint32_t value);
}

namespace Io {
    union [[gnu::packed]] RedirectEntry {
        struct {
            uint8_t vect;
            uint8_t delivery_mode : 3;
            uint8_t destination_mode : 1;
            uint8_t delivery_status : 1;
            uint8_t pin_polarity : 1;
            uint8_t remote_irr : 1;
            uint8_t trigger_mode : 1;
            uint8_t interrupt_mask : 1;
            uint64_t reserved : 39;
            uint8_t destination;
        };
        uint64_t raw;
    };

    void init();

    inline uint32_t read_reg(uint8_t offset);
    inline void write_reg(uint8_t offset, uint32_t value);
    void redirect_pin(uint8_t offset, uint8_t vect, uint8_t mask = 0);
}

}
