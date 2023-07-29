#include <kernel/apic.hpp>
#include <kernel/io.hpp>
#include <kernel/msr.hpp>
#include <kernel/terminal.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/int.hpp>
#include <kernel/timer.hpp>

namespace Apic {

void init() {
    Apic::disable_pic();

    Apic::Local::init();
    Apic::Io::init();
}

void disable_pic() {
    IoPort::out_port<uint8_t>(PIC_COMMAND_MASTER, ICW_1);
    IoPort::out_port<uint8_t>(PIC_COMMAND_SLAVE, ICW_1);
    IoPort::out_port<uint8_t>(PIC_DATA_MASTER, ICW_2_M);
    IoPort::out_port<uint8_t>(PIC_DATA_SLAVE, ICW_2_S);
    IoPort::out_port<uint8_t>(PIC_DATA_MASTER, ICW_3_M);
    IoPort::out_port<uint8_t>(PIC_DATA_SLAVE, ICW_3_S);
    IoPort::out_port<uint8_t>(PIC_DATA_MASTER, ICW_4);
    IoPort::out_port<uint8_t>(PIC_DATA_SLAVE, ICW_4);
    IoPort::out_port<uint8_t>(PIC_DATA_MASTER, 0xff);
    IoPort::out_port<uint8_t>(PIC_DATA_SLAVE, 0xff);
}

namespace Local {
    static uintptr_t apic_base_addr;

    // Enable the local APIC
    void init() {
        apic_base_addr = PHYS_TO_VIRT((uintptr_t)(read_msr(Msr::MsrType::ApicBase) & 0xfffff000));

        // Set the APIC_SW_EN bit in the SIVR to 1 and the IDT entry to 255
        uint64_t sivr = Local::read_reg(Local::Register::Siv);
        Local::write_reg(Local::Register::Siv, sivr | (1 << 8) | 255);
    }

    // TODO Make it inline
    uint32_t read_reg(Local::Register reg) {
        return *((volatile uint32_t *)(apic_base_addr + (uint32_t)reg));
    }

    // TODO Make it inline
    void write_reg(Local::Register reg, uint32_t value) {
        *((volatile uint32_t *)(apic_base_addr + (uint32_t)reg)) = value;
    }
}

namespace Io {
    static uintptr_t io_base_addr; 

    void init() {
        io_base_addr = PHYS_TO_VIRT(0xfec00000);

        // Redirect the pin corresponding to the PS/2 Keyboard
        Io::redirect_pin(0x12, KEYBOARD_VECT);

        // Configure the PIT to trigger an interrupt every 1 ms 
        uint16_t divisor = PIT_FREQUENCY / 1000;
        IoPort::out_port<uint8_t>(PIT_CMD, PIT_CHANNEL_0 | PIT_LOHI_ACCESS | PIT_MODE_2 | 
                PIT_BINARY_MODE);
        IoPort::out_port<uint8_t>(PIT_COUNTER_0, (uint8_t)divisor);
        IoPort::out_port<uint8_t>(PIT_COUNTER_0, (uint8_t)(divisor >> 8));

        // Redirect the pin corresponding to the timer
        Io::redirect_pin(0x14, TIMER_VECT);
        // Reset the local apic timer so that we can calculate the ticks passed between the 
        // interrupt triggered by the PIT and the current count of the local apic timer
        Local::write_reg(Local::Register::TimerInitCount, UINT32_MAX);  

        // Allows the processor to receive hardware interrupts
        Interrupt::enable_interrupts();
    }

    inline uint32_t read_reg(uint8_t offset) {
        *(volatile uint32_t*)io_base_addr = offset;
        return *(volatile uint32_t*)(io_base_addr + 0x10);
    }

    inline void write_reg(uint8_t offset, uint32_t value) {
        *(volatile uint32_t*)(io_base_addr) = offset;
        *(volatile uint32_t*)(io_base_addr + 0x10) = value;
    }

    void redirect_pin(uint8_t offset, uint8_t vect, uint8_t mask) {
        uint32_t lower = Io::read_reg(offset);
        uint32_t higher = Io::read_reg(offset + 1);

        RedirectEntry entry;
        entry.raw = ((uint64_t) higher << 32) | (uint64_t) lower;
        entry.vect = vect;
        entry.interrupt_mask = mask;

        lower = (uint32_t)entry.raw;
        higher = (uint32_t)(entry.raw >> 32);

        Io::write_reg(offset, lower);
        Io::write_reg(offset + 1, higher);
    }
}

}
