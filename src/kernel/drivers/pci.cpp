#include <kernel/drivers/pci.hpp>
#include <kernel/terminal.hpp>
#include <kernel/utils/list.hpp>
#include <kernel/drivers/e1000.hpp>

namespace Pci {

using DeviceInit = void (*)(Pci::Device pci_device, uint8_t bus, uint8_t device, uint8_t func);

static DeviceInit devices[] = {
    E1000::init
};

void init() {
    for (size_t bus = 0; bus < 256; bus++) {
        for (size_t device = 0; device < 32; device++) {
            size_t n_funcs = (Pci::read<uint8_t>(bus, device, 0, PCI_HEADER_OFFSET) & 0b10000000) 
                ? 8 : 1;

            for (size_t func = 0; func < n_funcs; func++) {
                uint16_t vendor = Pci::read<uint16_t>(bus, device, func, PCI_VENDOR_ID_OFFSET); 
                if (vendor == 0xffff) {
                    continue;
                }

                // Read the PCI header
                uint32_t header_bytes[sizeof(Pci::Header) / sizeof(uint32_t)] = { 0 };
                for (size_t reg = 0; reg < sizeof(header_bytes) / sizeof(uint32_t); reg++) {
                    header_bytes[reg] = 
                        Pci::read<uint32_t>(bus, device, func, reg * sizeof(uint32_t));
                }
                Pci::Header pci_header = *(Pci::Header*)&header_bytes;

                // Skip things like PCI-to-PCI bridges 
                if ((pci_header.header_type & 0x7f) != 0) {
                    continue;
                }

                // Read the PCI configuration
                uint32_t device_bytes[sizeof(Pci::Device) / sizeof(uint32_t)] = { 0 };
                for (size_t reg = 0; reg < sizeof(device_bytes) / sizeof(uint32_t); reg++) {
                    device_bytes[reg] = 
                        Pci::read<uint32_t>(bus, device, func, reg * sizeof(uint32_t));
                }
                Pci::Device pci_device = *(Pci::Device*)&device_bytes;

                for (const auto device_init : devices) {
                    device_init(pci_device, bus, device, func);
                }
            }
        }
    }
}

}
