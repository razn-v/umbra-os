#pragma once

#include <kernel/drivers/pci.hpp>
#include <kernel/int.hpp>
#include <kernel/net/socket.hpp>
#include <kernel/utils/ringbuffer.hpp>
#include <kernel/lock.hpp>
#include <stddef.h>

#define NUM_RX_DESCS   256
#define NUM_TX_DESCS   256

#define CTRL_REGISTER   0x0 
#define STATUS_REGISTER 0x8
#define EERD_REGISTER   0x14
#define ICR_REGISTER    0xc0
#define IMS_REGISTER    0xd0
#define IMC_REGISTER    0xd8
#define RCTL_REGISTER   0x100
#define TCTL_REGISTER   0x400
#define RDBAL_REGISTER  0x2800
#define RDBAH_REGISTER  0x2804
#define RDLEN_REGISTER  0x2808
#define RDH_REGISTER    0x2810
#define RDT_REGISTER    0x2818
#define TDBAL_REGISTER  0x3800
#define TDBAH_REGISTER  0x3804
#define TDLEN_REGISTER  0x3808
#define TDH_REGISTER    0x3810
#define TDT_REGISTER    0x3818

#define CTRL_DEVICE_RESET (1 << 26)
#define CTRL_SLU          (1 << 6)
#define RCTL_EN           (1 << 1)
#define RCTL_BAM          (1 << 15)
#define RCTL_SECRC        (1 << 26)
#define RCTL_BSIZE_256    (3 << 16)
#define RCTL_BSEX         (1 << 25)
#define TCTL_EN           (1 << 1)
#define IMS_TXDW          (1 << 0)
#define IMS_TXQE          (1 << 1)
#define IMS_LSC           (1 << 2)
#define IMS_RXSEQ         (1 << 3)
#define IMS_RXDMT0        (1 << 4)
#define IMS_RXO           (1 << 6)
#define IMS_RXT0          (1 << 7)
#define IMS_MDAC          (1 << 9)
#define IMS_RXCFG         (1 << 10)
#define IMS_PHYINT        (1 << 12)
#define IMS_GPI           0
#define IMS_GPI           0
#define IMS_TXD_LOW       0
#define IMS_SRPD          0
#define ICR_RXT0          (1 << 7)
#define TDESC_CMD_EOP     (1 << 0)
#define TDESC_CMD_IFCS    (1 << 1)
#define TDESC_CMD_RS      (1 << 3)
#define RDESC_STATUS_DD   (1 << 0)

struct [[gnu::packed]] TxDescriptor {
    uint64_t addr;
    uint16_t length;
    // Checksum offset
    uint8_t cso;
    uint8_t cmd;
    uint8_t reserved : 4;
    uint8_t status : 4;
    // Checksum start field
    uint8_t css;
    uint16_t special;
};

struct [[gnu::packed]] RxDescriptor {
    uint64_t addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
};

class E1000 {
public:
    static void init(Pci::Device pci_device, uint8_t bus, uint8_t device, uint8_t func);
    static E1000* get_driver();
    static void handler(Interrupt::Registers* reg);
    void send(void* data, size_t len);

    Socket::MacAddress mac_addr;
    Ringbuffer<Socket::Packet*, 64> cache;
    Spinlock cache_lock;

private:
    uint32_t* mmio;

    TxDescriptor* tx_descriptors;
    size_t tx_tail;
    RxDescriptor* rx_descriptors;
    size_t rx_tail;

    inline uint32_t read(uint32_t offset) {
        return this->mmio[offset / sizeof(uint32_t)];
    }

    inline void write(size_t reg, uint32_t value) {
        this->mmio[reg / sizeof(uint32_t)] = value;
    }

    void reset();
    void init_receive();
    void init_transmit();
    uint16_t read_eeprom(uint32_t addr);
};
