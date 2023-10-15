#include <kernel/net/socket.hpp>
#include <kernel/utils/ringbuffer.hpp>
#include <kernel/lock.hpp>

namespace Socket {

struct [[gnu::packed]] UdpHeader {
    be_uint16_t src_port;
    be_uint16_t dst_port;
    be_uint16_t length;
    be_uint16_t checksum;
    uint8_t data[];
};

struct UdpPacket {
    Socket::Ipv4Address src_ip;
    be_uint16_t src_port;
    size_t length;
    uint8_t* data;
};

class UdpSocket : public Socket::Handle {
public:
    Ringbuffer<UdpPacket*, 64> packets;
    Spinlock packets_lock;

    static void add_socket(Socket::UdpSocket* socket);
    static Socket::UdpSocket* get_socket(uint16_t port);
    static void receive_udp(Socket::Ipv4Header* ipv4_header);

    int send_msg(const struct msghdr* hdr, int flags, void* data, size_t len) override;
    int recv_msg(struct msghdr* hdr, int flags, void* data, size_t len) override;
};

}
