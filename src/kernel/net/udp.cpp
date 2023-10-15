#include <kernel/net/udp.hpp>
#include <kernel/libc/string.hpp>
#include <kernel/libc/misc.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/terminal.hpp>
#include <errno.h>
#include <netinet/in.h>
#include <fcntl.h>

namespace Socket {

static DoublyLinkedList<Socket::UdpSocket*> sockets;

void UdpSocket::add_socket(Socket::UdpSocket* socket) {
    sockets.push(socket);
}

Socket::UdpSocket* UdpSocket::get_socket(uint16_t port) {
    auto current = sockets.head;
    while (current != nullptr) {
        if (current->value->port == port) {
            return current->value;
        }
        current = current->next;
    }
    return nullptr;
}

// This is called when UDP packets are received by the network adapter. 
// Packets are then saved for future use.
void UdpSocket::receive_udp(Socket::Ipv4Header* ipv4_header) {
    // Make sure the packet is not too short
    size_t len = __builtin_bswap16(ipv4_header->length) - sizeof(Socket::Ipv4Header);
    if (len < sizeof(Socket::UdpHeader)) {
        return;
    } 

    auto udp_header = (Socket::UdpHeader*)ipv4_header->data;
    // Make sure the packet is not too long
    if (__builtin_bswap16(udp_header->length) > len) {
        return;
    } 

    Socket::UdpSocket* socket = UdpSocket::get_socket(__builtin_bswap16(udp_header->dst_port));
    if (socket == nullptr) {
        return;
    }

    auto packet = new Socket::UdpPacket;
    packet->src_ip = ipv4_header->src;
    packet->src_port = udp_header->src_port;
    packet->length = __builtin_bswap16(udp_header->length);
    packet->data = new uint8_t[packet->length];
    memcpy(packet->data, udp_header->data, packet->length);

    socket->packets_lock.acquire();
    socket->packets.write(packet);
    socket->packets_lock.release();
    Scheduler::wake_net();
}

int UdpSocket::send_msg(const struct msghdr* hdr, [[gnu::unused]] int flags, void* data, 
        size_t len) {
    if (hdr->msg_name == nullptr || hdr->msg_namelen <= 0) {
        return -ENOSYS;
    }

    auto addr = (sockaddr_in*)hdr->msg_name;
    if (addr->sin_family != PF_INET) {
        return -ENOSYS;
    }

    // TODO: Make a port allocator
    this->port = 1337;

    auto header = (Socket::UdpHeader*)(new uint8_t[sizeof(Socket::UdpHeader) + len]);
    memset(header, 0, sizeof(Socket::UdpHeader) + len);
    header->src_port = __builtin_bswap16(this->port); 
    header->dst_port = addr->sin_port; 
    header->length = __builtin_bswap16(sizeof(Socket::UdpHeader) + len);
    header->checksum = 0;
    memcpy(header->data, data, len);

    int res = Socket::send_ipv4(header, sizeof(Socket::UdpHeader) + len, IPPROTO_UDP, 
            Ipv4Address(0, 0, 0, 0), Ipv4Address(192, 168, 1, 254));
    if (res < 0) {
        delete header;
        return res;
    }
    
    delete header;
    return len;
}

int UdpSocket::recv_msg(struct msghdr* hdr, int flags, void* data, size_t len) {
    this->packets_lock.acquire();
    if (this->packets.is_empty()) {
        this->packets_lock.release();

        if (flags & MSG_DONTWAIT) {
            return -EAGAIN;
        } else if ((flags & O_NONBLOCK) != 0) {
            return -EWOULDBLOCK;
        }

        Scheduler::block_on([this]() { return !this->packets.is_empty(); });
    }

    Socket::UdpPacket* packet = this->packets.read().unwrap();
    this->packets_lock.release();

    if (hdr->msg_name != nullptr) {
        sockaddr_in addr;
        addr.sin_family = PF_INET;
        addr.sin_port = packet->src_port;
        addr.sin_addr.s_addr = packet->src_ip.value;

        memcpy(hdr->msg_name, &addr, sizeof(sockaddr_in));
        hdr->msg_namelen = sizeof(sockaddr_in);
    }

    size_t data_length = MIN(len, packet->length);
    memcpy(data, packet->data, data_length);
    delete packet->data;
    delete packet;

    return data_length;
}

}
