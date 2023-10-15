#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "ip_icmp.h"

#define PING_PKT_S 64
#define PORT_NO 0
#define PING_SLEEP_RATE 1000000
#define RECV_TIMEOUT 1

struct ping_pkt {
    struct icmphdr hdr;
    char msg[PING_PKT_S - sizeof(struct icmphdr)];
};

unsigned short checksum(void* b, int len) {
    unsigned short* buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }
    
    if (len == 1) {
        sum += *(unsigned char*)buf;
    }
    
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

char* dns_lookup(char* addr_host, struct sockaddr_in* addr_con) {
    printf("\nResolving DNS..\n");
    struct hostent* host_entity;
    char* ip = (char*)malloc(NI_MAXHOST * sizeof(char));

    if ((host_entity = gethostbyname(addr_host)) == NULL) {
        return NULL;
    }

    strcpy(ip, inet_ntoa(*(struct in_addr*)host_entity->h_addr));

    (*addr_con).sin_family = host_entity->h_addrtype;
    (*addr_con).sin_port = htons(PORT_NO);
    (*addr_con).sin_addr.s_addr = *(long*)host_entity->h_addr;

    return ip;
}

void send_ping(int ping_sockfd, struct sockaddr_in* ping_addr, char* ping_ip, char* rev_host) {
    int ttl_val = 64, msg_count = 0, i, flag = 1; 
    socklen_t addr_len;
    struct ping_pkt pckt;
    struct sockaddr_in r_addr;

    while (1) {
        flag = 1;
        bzero(&pckt, sizeof(pckt));
        pckt.hdr.type = ICMP_ECHO;

        for (i = 0; i < sizeof(pckt.msg) - 1; i++) {
            pckt.msg[i] = i + '0';
        }

        pckt.msg[i] = 0;
        pckt.hdr.un.echo.sequence = msg_count++;
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

        usleep(PING_SLEEP_RATE);

        if (sendto(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*)ping_addr, 
                    sizeof(*ping_addr)) <= 0) {
            printf("\nPacket Sending Failed!\n");
            flag = 0;
        }

        addr_len = sizeof(r_addr);

        if (recvfrom(ping_sockfd, &pckt, sizeof(pckt), 0,
            (struct sockaddr*)&r_addr, &addr_len) <= 0
            && msg_count > 1) {
            printf("\nPacket receive failed!\n");
        } else if (flag) {
            if (!(pckt.hdr.type == ICMP_ECHOREPLY && pckt.hdr.code == 0)) {
                printf("Error..Packet received with ICMP type %d code %d\n", pckt.hdr.type, 
                        pckt.hdr.code);
            } else {
                printf("%d bytes from (h: %s) (%s) msg_seq=%d ttl=%d.\n", PING_PKT_S, rev_host,
                    ping_ip, msg_count, ttl_val);
            }
        }
    }
}

int main() {
    int sockfd;
    char* ip_addr;
    struct sockaddr_in addr_con;

    int argc = 2;
    char* argv[] = {"", "google.com"};

    if (argc != 2) {
        printf("\nFormat %s <address>\n", argv[0]);
        return 0;
    }

    ip_addr = dns_lookup(argv[1], &addr_con);
    if (ip_addr == NULL) {
        printf("\nDNS lookup failed! Could not resolve hostname!\n");
        return 0;
    }

    printf("\nTrying to connect to '%s' IP: %s\n", argv[1], ip_addr);

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        printf("\nSocket file descriptor not received!!\n");
        return 0;
    }
    else {
        printf("\nSocket file descriptor %d received\n", sockfd);
    }

    send_ping(sockfd, &addr_con, ip_addr, argv[1]);

    return 0;
}
