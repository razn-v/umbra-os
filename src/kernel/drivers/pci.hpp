#pragma once

#include <kernel/io.hpp>

#define PCI_CONFIG_ADDRESS    0xcf8
#define PCI_CONFIG_DATA       0xcfc
#define PCI_ADDRESS_ENABLE    1 << 31

#define PCI_VENDOR_ID_OFFSET  0x0
#define PCI_DEVICE_ID_OFFSET  0x2
#define PCI_SUBCLASS_OFFSET   0xa
#define PCI_CLASS_CODE_OFFSET 0xb
#define PCI_HEADER_OFFSET     0xe

namespace Pci {

struct [[gnu::packed]] Header {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class_code;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;
};

struct [[gnu::packed]] Device {
    Header header;
    uint32_t bar0;
    uint32_t bar1;
    uint32_t bar2;
    uint32_t bar3;
    uint32_t bar4;
    uint32_t bar5;
    uint32_t cardbus_ptr;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_device_id;
    uint32_t expansion_rom_addr;
    uint8_t capabilities;
    uint8_t reserved[7];
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;
};

class DeviceHandler {
public:
    virtual void init(Pci::Device device) = 0;
};

void init();

template <typename T>
T read(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t addr = PCI_ADDRESS_ENABLE | (bus << 16) | (device << 11) | (func << 8) | 
        (offset & 0xfc);

    IoPort::out_port<uint32_t>(PCI_CONFIG_ADDRESS, addr);

    // Determine which word of the register will be chosen
    offset = (offset & 0b11) * 8;
    return (T)(IoPort::in_port<uint32_t>(PCI_CONFIG_DATA) >> offset);
}

template <typename T>
void write(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t addr = PCI_ADDRESS_ENABLE | (bus << 16) | (device << 11) | (func << 8) | 
        (offset & 0xfc);
    uint32_t current = Pci::read<uint32_t>(bus, device, func, offset);

    IoPort::out_port<uint32_t>(PCI_CONFIG_ADDRESS, addr);

    uint32_t mask;
    if constexpr (sizeof(T) == 1) {
        mask = ~((uint32_t)0xff << offset);
        value = (current & mask) | ((value & 0xff) << offset);
    } else if constexpr (sizeof(T) == 2) {
        offset = (offset & 0b11) * 8;
        mask = ~((uint32_t)0xffff << offset);
        value = (current & mask) | ((value & 0xffff) << offset);
    }

    IoPort::out_port<uint32_t>(PCI_CONFIG_DATA, value);
}

}
