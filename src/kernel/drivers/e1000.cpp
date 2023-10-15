#include <kernel/drivers/e1000.hpp>
#include <kernel/terminal.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/drivers/pci.hpp>
#include <kernel/apic.hpp>
#include <kernel/int.hpp>
#include <kernel/scheduler.hpp>

static E1000* e1000 = nullptr;

E1000* E1000::get_driver() {
    return e1000;
}

void E1000::handler([[gnu::unused]] Interrupt::Registers* regs) {
    uint32_t status = e1000->read(ICR_REGISTER);

    // Check if we received a packet
    if (status & ICR_RXT0) {
        // Loop until the entire packet is dumped
        while (true) {
            e1000->rx_tail = e1000->read(RDT_REGISTER);
            // Check if the queue is full
            if (e1000->rx_tail == e1000->read(RDH_REGISTER)) {
                return;
            }

            e1000->rx_tail = (e1000->rx_tail + 1) % NUM_RX_DESCS;
            RxDescriptor desc = e1000->rx_descriptors[e1000->rx_tail];

            // Break if the descriptor is not done
            if (!(desc.status & RDESC_STATUS_DD)) {
                break;
            }

            // Reset the status
            desc.status = 0;

            if (desc.length <= MAX_ETHERNET_FRAME_SIZE) {
                Socket::Packet* packet = new Socket::Packet;
                packet->length = desc.length; 
                packet->data = new uint8_t[packet->length];
                memcpy(packet->data, (void*)PHYS_TO_VIRT(desc.addr), packet->length);

                e1000->cache.write(packet);
                Scheduler::wake_net();
            }

            // Update the tail
            e1000->write(RDT_REGISTER, e1000->rx_tail);
        }
    }
}

void E1000::init(Pci::Device pci_device, uint8_t bus, uint8_t device, uint8_t func) {
    if (pci_device.header.vendor_id != 0x8086 || pci_device.header.device_id != 0x100e) {
        return;
    }

    // Enable bus mastering
    Pci::write<uint16_t>(bus, device, func, 0x4, 
            Pci::read<uint16_t>(bus, device, func, 0x4) | (1 << 2));
    // Enable mmio
    Pci::write<uint16_t>(bus, device, func, 0x4, 
            Pci::read<uint16_t>(bus, device, func, 0x4) | (1 << 1));

    e1000 = new E1000;
    e1000->tx_tail = 0;
    e1000->rx_tail = 0;

    // Check the address space size
    uint32_t size = (pci_device.bar0 >> 1) & 0b11;
    if (size == 0) {
        // 32-bit
        e1000->mmio = (uint32_t*)PHYS_TO_VIRT((uintptr_t)(pci_device.bar0 & 0xffff'fff0));
    } else {
        // 64-bit
        uint64_t low_addr = pci_device.bar0 & 0xffff'fff0;
        e1000->mmio = (uint32_t*)PHYS_TO_VIRT(((uint64_t)pci_device.bar1) << 32 | low_addr);
    }

    e1000->reset();

    // Enable link
    e1000->write(CTRL_REGISTER, e1000->read(CTRL_REGISTER) | CTRL_SLU);
    while ((e1000->read(STATUS_REGISTER) & 2) != 2) {}

    e1000->init_receive();
    e1000->init_transmit();

    uint8_t pci_irq = Pci::read<uint8_t>(bus, device, func, 0x3c);
    Apic::Io::redirect_pin(pci_irq * 2 + 16, E1000_VECT);

    // Enable interrupts
    e1000->write(IMS_REGISTER, 0x1F6DF);

    Terminal::printf("{green}[*]{white} E1000 initialized.\n");

    for (size_t i = 0; i < 3; i++) {
        uint16_t word = e1000->read_eeprom(i);
        e1000->mac_addr.data[i * 2] = word & 0xff;
        e1000->mac_addr.data[i * 2 + 1] = word >> 8;
    }

    Scheduler::add_task(Task::create("packet_handler", Socket::packet_handler));
}

void E1000::reset() {
    // Disable all interrupts
    this->write(IMC_REGISTER, 0xffff'ffff); 

    // Issue a global reset
    this->write(CTRL_REGISTER, this->read(CTRL_REGISTER) | CTRL_DEVICE_RESET);

    // Wait 1ms before checking if the bit has cleared
    Scheduler::sleep(1);
    while ((this->read(CTRL_REGISTER) & CTRL_DEVICE_RESET) != 0) {};

    // Disable all interrupts
    this->write(IMC_REGISTER, 0xffff'ffff);
}

void E1000::init_receive() {
    this->rx_descriptors = (RxDescriptor*)PHYS_TO_VIRT((uintptr_t)Pmm::calloc(1));

    for (size_t i = 0; i < NUM_RX_DESCS; i++) {
        this->rx_descriptors[i].addr = (uint64_t)Pmm::calloc(1);
    }

    this->write(RDBAL_REGISTER, VIRT_TO_PHYS((uintptr_t)rx_descriptors) & 0xffff'ffff);
    this->write(RDBAH_REGISTER, VIRT_TO_PHYS((uintptr_t)rx_descriptors) >> 32);

    this->write(RDLEN_REGISTER, PAGE_SIZE);
    this->write(RDH_REGISTER, 0);
    this->write(RDT_REGISTER, NUM_RX_DESCS - 1);

    // Enable receiver, accept broadcast packets, strip ethern CRC, set the receive buffer size to
    // 256 * 16 (BSEX) = 4096 bytes
    this->write(RCTL_REGISTER, RCTL_EN | RCTL_BAM | RCTL_SECRC | RCTL_BSIZE_256 | RCTL_BSEX);
}

void E1000::init_transmit() {
    this->tx_descriptors = (TxDescriptor*)PHYS_TO_VIRT((uintptr_t)Pmm::calloc(1));

    for (size_t i = 0; i < NUM_TX_DESCS; i++) {
        this->tx_descriptors[i].addr = (uint64_t)Pmm::calloc(1);
    }

    this->write(TDBAL_REGISTER, VIRT_TO_PHYS((uintptr_t)tx_descriptors) & 0xffff'ffff);
    this->write(TDBAH_REGISTER, VIRT_TO_PHYS((uintptr_t)tx_descriptors) >> 32);

    this->write(TDLEN_REGISTER, PAGE_SIZE);
    this->write(TDH_REGISTER, 0);
    this->write(TDT_REGISTER, 0);

    // Enable transmitter
    this->write(TCTL_REGISTER, TCTL_EN);
}

uint16_t E1000::read_eeprom(uint32_t addr) {
    uint32_t ret = 0;
    this->write(EERD_REGISTER, 1 | (addr << 8));

    while (!((ret = this->read(EERD_REGISTER)) & (1 << 4)));

    return (uint16_t)((ret >> 16) & 0xffff);
}

void E1000::send(void* data, size_t len) {
    bool restore = Interrupt::interrupts_enabled();
    Interrupt::disable_interrupts();

    TxDescriptor* desc = &this->tx_descriptors[this->tx_tail];
    memcpy((void*)PHYS_TO_VIRT(desc->addr), data, len);
    desc->length = len;
    // End Of Packet, Insert FCS and Report Status
    desc->cmd = TDESC_CMD_EOP | TDESC_CMD_IFCS | TDESC_CMD_RS;

    this->tx_tail = (this->tx_tail + 1) % NUM_TX_DESCS;
    this->write(TDT_REGISTER, this->tx_tail);

    if (restore) {
        Interrupt::enable_interrupts();
    }
}
