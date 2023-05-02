#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

#define PACKETSIZE  64
#define MAXWAIT     5
#define TIMEOUT     1

struct packet {
    struct icmphdr hdr;
    char msg[PACKETSIZE - sizeof(struct icmphdr)];
};

int ping(char *address) {
    struct packet pckt;
    struct sockaddr_in r_addr;
    int sfd, i, status, attempts = 0, recv_len;
    unsigned int len = sizeof(r_addr);
    struct timeval tv;

    if ((sfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("socket");
        return -1;
    }

    memset(&r_addr, 0, sizeof(r_addr));
    r_addr.sin_family = AF_INET;
    r_addr.sin_addr.s_addr = inet_addr(address);

    memset(&pckt, 0, sizeof(pckt));
    pckt.hdr.type = ICMP_ECHO;
    pckt.hdr.un.echo.id = getpid();

    for (i = 0; i < sizeof(pckt.msg); i++) {
        pckt.msg[i] = i + '0';
    }

    while (attempts < MAXWAIT) {
        if (sendto(sfd, &pckt, sizeof(pckt), 0, (struct sockaddr *)&r_addr, sizeof(r_addr)) <= 0) {
            perror("sendto");
            return -1;
        }

        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sfd, &readfds);

        if (select(sfd + 1, &readfds, NULL, NULL, &tv) <= 0) {
            printf("Timeout for %s\n", address);
            attempts++;
            continue;
        }

        if ((recv_len = recvfrom(sfd, &pckt, sizeof(pckt), 0, (struct sockaddr *)&r_addr, &len)) < 0) {
            perror("recvfrom");
            return -1;
        }

        if (pckt.hdr.type == ICMP_ECHOREPLY) {
            printf("%s is UP\n", address);
            status = 1;
            break;
        }

        attempts++;
    }

    close(sfd);
    return status;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s [IP address]\n", argv[0]);
        return -1;
    }

    char *address = argv[1];
    if (ping(address) < 0) {
        printf("%s is DOWN\n", address);
        return -1;
    }

    return 0;
}
