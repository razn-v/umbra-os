#pragma once

#include <sys/socket.h>
#include <stdint.h>
#include <kernel/utils/endian.hpp>

// The maximum ethernet frame size is 1518 bytes
#define MAX_ETHERNET_FRAME_SIZE 1518

namespace Socket {

struct [[gnu::packed]] MacAddress {
    uint8_t data[6];

    MacAddress() = default;

    MacAddress(uint8_t part1, uint8_t part2, uint8_t part3, uint8_t part4, uint8_t part5, 
            uint8_t part6) {
        data[0] = part1;
        data[1] = part2;
        data[2] = part3;
        data[3] = part4;
        data[4] = part5;
        data[5] = part6;
    }
};

union Ipv4Address {
    struct {
        uint8_t data[4];
    };
    uint32_t value;

    Ipv4Address() = default;

    Ipv4Address(uint8_t part1, uint8_t part2, uint8_t part3, uint8_t part4) {
        data[0] = part1;
        data[1] = part2;
        data[2] = part3;
        data[3] = part4;
    }

    Ipv4Address(uint32_t addr) : value(addr) {}
};

enum EtherType {
    Ipv4 = 0x800,
    Arp = 0x806
};

enum Ipv4Protocol {
    Icmp = 0x1,
    Tcp = 0x6,
    Udp = 0x11
}; 

struct [[gnu::packed]] EthernetFrame {
    MacAddress dst;
    MacAddress src;
    be_uint16_t ether_type;
    uint8_t data[];
};

struct [[gnu::packed]] Ipv4Header {
    // Internet Header Length
    uint8_t ihl : 4;
    uint8_t version : 4;
    // Explicit Congestion Notification
    uint8_t ecn : 2;
    // Differentiated Services Code Point
    uint8_t dscp : 6;
    be_uint16_t length;
    be_uint16_t id;
    be_uint16_t fragment_offset : 13;
    // Time to Live
    uint8_t ttl;
    uint8_t protocol;
    be_uint16_t checksum;
    Ipv4Address src;
    Ipv4Address dst;
    uint8_t data[];
};

struct [[gnu::packed]] ArpHeader {
    // Hardware Type
    be_uint16_t hw_type;
    // Protocol Type
    be_uint16_t pr_type;
    // Hardware Length
    uint8_t hw_length;
    // Protocol Length
    uint8_t pr_length;
    // Operation Code
    be_uint16_t opcode;
    // Sender Hardware Address
    MacAddress src_mac;
    // Sender Protocol Address
    Ipv4Address src_ip;
    // Target Hardware Address
    MacAddress dst_mac;
    // Target Protocol Address
    Ipv4Address dst_ip;
};

struct Packet {
    uint8_t* data;
    size_t length;
};

class Handle {
public:
    int id = -1;
    uint16_t port;

    virtual int send_msg(const struct msghdr* hdr, int flags, void* data, size_t len) = 0;
    virtual int recv_msg(struct msghdr* hdr, int flags, void* data, size_t len) = 0;
};

int send_ipv4(void* data, size_t len, uint8_t protocol, Ipv4Address src, Ipv4Address dst);
void receive_ipv4(void* data, size_t len);
be_uint16_t get_checksum(void* data, size_t size);

[[gnu::noreturn]] void packet_handler();

}
