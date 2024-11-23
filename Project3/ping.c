#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

unsigned short checksum(unsigned short *buf, int len);
void ping(const char *ipaddr);

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ip address>\n", argv[0]);
        exit(1);
    }
    ping(argv[1]);
    return 0;
}

void ping(const char *ipaddr) {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipaddr);

    int seq = 0;
    while (1) {
        struct icmphdr icmp_hdr;
        icmp_hdr.type = ICMP_ECHO;
        icmp_hdr.code = 0;
        icmp_hdr.checksum = 0;
        icmp_hdr.un.echo.id = getpid();
        icmp_hdr.un.echo.sequence = seq++;
        icmp_hdr.checksum = checksum((unsigned short *)&icmp_hdr, sizeof(icmp_hdr));

        struct timeval send_time;
        gettimeofday(&send_time, NULL);
        if (sendto(sockfd, &icmp_hdr, sizeof(icmp_hdr), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("sendto");
            exit(1);
        }

        char recv_buf[1024];
        struct sockaddr_in recv_addr;
        socklen_t addr_len = sizeof(recv_addr);
		ssize_t n;
        if ((n = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&recv_addr, &addr_len)) < 0) {
            perror("recvfrom");
            exit(1);
        }
		
		int iphlen, icmplen;
		struct ip *ip = (struct ip *) recv_buf;

		iphlen = ip->ip_hl << 2;
		icmplen = n - iphlen;

        struct icmphdr *icmp_reply = (struct icmphdr *)(recv_buf + sizeof(struct iphdr));
        if (icmp_reply->type == ICMP_ECHOREPLY && icmp_reply->un.echo.id == getpid()) {
            struct timeval recv_time;
            gettimeofday(&recv_time, NULL);
            double rtt = (recv_time.tv_sec - send_time.tv_sec) * 1000.0 + (recv_time.tv_usec - send_time.tv_usec) / 1000.0;
            printf("%d bytes from %s: icmp_seq=%d, ttl=%d, rtt=%.3fms\n", icmplen, inet_ntoa(ip->ip_src), icmp_reply->un.echo.sequence, ip->ip_ttl, rtt);
		}

        sleep(1);
    }
}

unsigned short checksum(unsigned short *buf, int len) {
    unsigned int sum = 0;
    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }
    if (len == 1) {
        sum += (*buf & 0xFF00);
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}
