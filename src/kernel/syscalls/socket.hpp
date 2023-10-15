#include <sys/socket.h>

int sys_socket(int family, int type, int protocol);
int sys_send_msg(int sockfd, const struct msghdr* hdr, int flags);
int sys_recv_msg(int sockfd, struct msghdr* hdr, int flags);
int sys_set_sock_opt(int fd, int layer, int number, const void* buffer, socklen_t size);
