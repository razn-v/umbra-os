#include <kernel/net/icmp.hpp>
#include <kernel/utils/list.hpp>
#include <kernel/libc/string.hpp>
#include <kernel/terminal.hpp>
#include <kernel/scheduler.hpp>
#include <errno.h>
#include <netinet/in.h>
#include <fcntl.h>

namespace Socket {

static DoublyLinkedList<Socket::IcmpSocket*> sockets;

void IcmpSocket::add_socket(Socket::IcmpSocket* socket) {
    sockets.push(socket);
}

void IcmpSocket::receive_icmp(Socket::Ipv4Header* ipv4_header) {
    // Make sure the packet is not too short
    size_t len = __builtin_bswap16(ipv4_header->length) - sizeof(Ipv4Header);
    if (len < sizeof(Socket::IcmpHeader)) {
        return;
    } 

    Socket::IcmpHeader* icmp_header = (IcmpHeader*)ipv4_header->data;

    // Echo reply
    if (icmp_header->type == 0 && icmp_header->code == 0) {
        // Write the packet for each socket
        auto current = sockets.head;
        while (current != nullptr) {
            current->value->packets_lock.acquire();
            current->value->packets.write(icmp_header);
            current->value->packets_lock.release();
            current = current->next;
        }

        Scheduler::wake_net();
    }
}

int IcmpSocket::send_msg(const struct msghdr* hdr, [[gnu::unused]] int flags, void* data, 
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

    IcmpHeader* header = (IcmpHeader*)(new uint8_t[sizeof(IcmpHeader) + len]);
    header->type = 8;
    header->code = 0;
    header->checksum = 0;
    memcpy(header->data, data, len);
    header->checksum = Socket::get_checksum(header, sizeof(IcmpHeader) + len);

    int res = Socket::send_ipv4(header, sizeof(IcmpHeader) + len, IPPROTO_ICMP, 
            Ipv4Address(0, 0, 0, 0), addr->sin_addr.s_addr);
    if (res < 0) {
        delete header;
        return res;
    }
    
    delete header;
    return len;
}

int IcmpSocket::recv_msg([[gnu::unused]] struct msghdr* hdr, int flags, void* data, size_t len) {
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

    Socket::IcmpPacket* packet = this->packets.read().unwrap();
    this->packets_lock.release();

    memcpy(data, packet, len);

    return len;
}

}
