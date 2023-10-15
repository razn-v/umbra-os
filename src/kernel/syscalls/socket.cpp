#include <kernel/syscalls/socket.hpp>
#include <kernel/terminal.hpp>
#include <kernel/net/socket.hpp>
#include <kernel/net/udp.hpp>
#include <kernel/net/icmp.hpp>
#include <kernel/scheduler.hpp>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>

int sys_socket(int family, int type, int protocol) {
    auto task = Scheduler::get_current_task();

    switch (family) {
        case AF_UNIX: {
            Terminal::printf("{red}(socket) Not implemented.");
            return -ENOSYS;
        }
        case AF_INET: {
            switch (type) {
                case SOCK_STREAM:
                    Terminal::printf("{red}(socket) Not implemented.");
                    return -ENOSYS;
                case SOCK_DGRAM:
                    if (protocol == 0 || protocol == Socket::Ipv4Protocol::Udp) {
                        auto socket = new Socket::UdpSocket;
                        Socket::UdpSocket::add_socket(socket);
                        return task->add_socket_handle(socket);
                    } else {
                        Terminal::printf("{red}(socket) Not implemented.");
                        return -ENOSYS;
                    }
                    break;
                case SOCK_RAW:
                    if (protocol == Socket::Ipv4Protocol::Icmp) {
                        auto socket = new Socket::IcmpSocket;
                        Socket::IcmpSocket::add_socket(socket);
                        return task->add_socket_handle(socket);
                    } else {
                        return -ENOSYS;
                    }
                    break;
                default:
                    Terminal::printf("{red}(socket) Not implemented.");
                    return -ENOSYS;
            }
            break;
        }
        default: {
            Terminal::printf("{red}(socket) Not implemented.");
            return -ENOSYS;
        }
    }

    return -1;
}

int sys_send_msg(int sockfd, const struct msghdr* hdr, int flags) {
    auto task = Scheduler::get_current_task();

    Socket::Handle* socket = task->get_socket_handle(sockfd); 
    if (socket == nullptr) {
        return -1;
    }

    int sent = 0;
    for (size_t i = 0; i < hdr->msg_iovlen; i++) {
        int res = socket->send_msg(hdr, flags, hdr->msg_iov[i].iov_base, hdr->msg_iov[i].iov_len);
        if (res < 0) {
            return res;
        }
        sent += res;
    }

    return sent;
}

int sys_recv_msg(int sockfd, struct msghdr* hdr, int flags) {
    auto task = Scheduler::get_current_task();

    Socket::Handle* socket = task->get_socket_handle(sockfd); 
    if (socket == nullptr) {
        return -1;
    }

    size_t read = 0;
    for (size_t i = 0; i < hdr->msg_iovlen; i++) {
        int res = socket->recv_msg(hdr, flags, hdr->msg_iov[i].iov_base, hdr->msg_iov[i].iov_len);
        if (res < 0) {
            return res;
        }
        read += res;
    }

    return read;
}

int sys_set_sock_opt([[gnu::unused]] int fd, [[gnu::unused]] int layer, [[gnu::unused]] int number, 
        [[gnu::unused]] const void* buffer, [[gnu::unused]] socklen_t size) {
    // TODO
    return 0;
}
