#include <kernel/net/socket.hpp>
#include <kernel/libc/string.hpp>
#include <kernel/terminal.hpp>
#include <kernel/drivers/e1000.hpp>
#include <kernel/net/udp.hpp>
#include <kernel/net/icmp.hpp>
#include <kernel/scheduler.hpp>
#include <netinet/in.h>

namespace Socket {

int send_ipv4(void* data, size_t len, uint8_t protocol, Socket::Ipv4Address src, 
        Socket::Ipv4Address dst) {
    E1000* driver = E1000::get_driver();
    if (driver == nullptr) {
        return -1;
    }

    auto frame = (Socket::EthernetFrame*)(new uint8_t[
            sizeof(Socket::EthernetFrame) + sizeof(Socket::Ipv4Header) + len]);
    frame->ether_type = __builtin_bswap16(Socket::EtherType::Ipv4);
    frame->src = driver->mac_addr;

    // FIXME: Should not be always broadcast
    frame->dst = MacAddress(0xff, 0xff, 0xff, 0xff, 0xff, 0xff);

    auto header = (Socket::Ipv4Header*)frame->data;
    memset(header, 0, sizeof(Socket::Ipv4Header));
    
    // 5 dwords = 5*4 = 20 bytes
    header->ihl = 5;
    // IPv4
    header->version = 4;
    header->length = __builtin_bswap16(len + sizeof(Socket::Ipv4Header));
    // As recommended by RFC 17002
    header->ttl = 64;
    header->protocol = protocol;
    header->dst = dst;
    header->src = src;
    header->checksum = Socket::get_checksum(header, sizeof(Socket::Ipv4Header));

    memcpy(header->data, data, len);
    driver->send(frame, sizeof(Socket::EthernetFrame) + sizeof(Socket::Ipv4Header) + len);

    return 0;
}

void receive_ipv4(void* data, size_t len) {
    if (len < sizeof(Socket::Ipv4Header)) {
        // Packet too short
        return;
    }

    Socket::Ipv4Header* header = (Socket::Ipv4Header*)data;
    // Make sure the packet is actually IPv4
    if (header->version != 4) {
        return;
    }

    be_uint16_t checksum = header->checksum;
    // Check if the checksum is valid. 
    // Make sure the `checksum` field is not taken into account first.
    header->checksum = 0;
    if (checksum != Socket::get_checksum(data, sizeof(Ipv4Header))) {
        return;
    }

    switch (header->protocol) {
        case Socket::Ipv4Protocol::Icmp:
            IcmpSocket::receive_icmp(header);
            break;
        case Socket::Ipv4Protocol::Tcp:
            // TODO
            break;
        case Socket::Ipv4Protocol::Udp:
            UdpSocket::receive_udp(header);
            break;
        default:
            break;
    }
}

be_uint16_t get_checksum(void* data, size_t size) {
    uint16_t* ptr = (uint16_t*)data;
    uint32_t checksum = 0;

    for (size_t i = size; i >= 2; i -= 2) {
        checksum += *ptr++;
    }

    checksum = (checksum & 0xffff) + (checksum >> 16);
    if (checksum > UINT16_MAX) {
        checksum += 1;
    }

    return ~checksum;
}

[[gnu::noreturn]] void packet_handler() {
    E1000* driver = E1000::get_driver();
    while (true) {
        // Block the thread until we receive a packet
        Scheduler::block_on([driver]() { return !driver->cache.is_empty(); });

        driver->cache_lock.acquire();

        Socket::Packet* packet = driver->cache.read().unwrap();
        Socket::EthernetFrame* frame = (EthernetFrame*)packet->data;

        switch (__builtin_bswap16(frame->ether_type)) {
            case Socket::EtherType::Ipv4:
                Socket::receive_ipv4(frame->data, packet->length - sizeof(Socket::EthernetFrame));
                break;
            case Socket::EtherType::Arp:
                // TODO
                break;
        }

        delete packet->data;
        delete packet;

        driver->cache_lock.release();
    }
}

}
