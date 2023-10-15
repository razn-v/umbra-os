#include <kernel/net/socket.hpp>
#include <kernel/utils/ringbuffer.hpp>
#include <kernel/lock.hpp>

namespace Socket {

struct [[gnu::packed]] IcmpHeader {
    uint8_t type;
    uint8_t code;
    be_uint16_t checksum;
    uint8_t data[];
};

using IcmpPacket = IcmpHeader;

class IcmpSocket : public Socket::Handle {
public:
    Ringbuffer<IcmpPacket*, 64> packets;
    Spinlock packets_lock;

    static void add_socket(Socket::IcmpSocket* socket);
    static void receive_icmp(Socket::Ipv4Header* ipv4_header);

    int send_msg(const struct msghdr* hdr, int flags, void* data, size_t len) override;
    int recv_msg(struct msghdr* hdr, int flags, void* data, size_t len) override;
};

}
