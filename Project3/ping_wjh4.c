#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>
#include <ctype.h>

#define PACKET_SIZE 64
#define MAX_COUNT 1000 // count的最大允许值

// 全局统计信息
static int packets_sent = 0;
static int packets_received = 0;
static int count = 0;          // 发送的ICMP包数量，默认0表示无限制
static int infinite_mode = 1; // 是否为无限模式

// 时间差计算
double time_diff(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 + (end->tv_usec - start->tv_usec) / 1000.0;
}

// 计算校验和
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// 判断字符串是否为数字
int is_numeric(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(str[i]))
            return 0;
    }
    return 1;
}

// 打印统计信息
void print_statistics() {
    printf("\n--- Ping statistics ---\n");
    printf("%d packets transmitted, %d packets received, %.2f%% packet loss\n",
           packets_sent, packets_received,
           packets_sent > 0 ? ((packets_sent - packets_received) * 100.0 / packets_sent) : 0.0);
}

// 信号处理函数，用于捕获Ctrl+C
void handle_sigint(int sig) {
    (void)sig; // 忽略未使用的参数
    print_statistics();
    exit(0);
}

// 打印用法信息
void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-c <count>] <destination_ip>\n", prog_name);
    fprintf(stderr, "  -c <count>  Specify the number of ICMP requests to send (1-1000)\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: Missing destination IP or hostname.\n");
        print_usage(argv[0]);
    }

    char *destination_ip = NULL;
    int count_specified = 0;

    // 注册Ctrl+C信号处理
    signal(SIGINT, handle_sigint);

    // 解析参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 >= argc || !is_numeric(argv[i + 1])) {
                fprintf(stderr, "Error: -c option requires a valid positive integer argument.\n");
                print_usage(argv[0]);
            }
            count = atoi(argv[++i]);
            count_specified = 1;
            infinite_mode = 0;

            if (count <= 0 || count > MAX_COUNT) {
                fprintf(stderr, "Error: Count value must be between 1 and %d.\n", MAX_COUNT);
                exit(EXIT_FAILURE);
            }
        } else if (destination_ip == NULL) {
            destination_ip = argv[i];
        } else {
            fprintf(stderr, "Error: Unrecognized argument: %s\n", argv[i]);
            print_usage(argv[0]);
        }
    }

    if (destination_ip == NULL) {
        fprintf(stderr, "Error: Missing destination IP or hostname.\n");
        print_usage(argv[0]);
    }

    int sockfd;
    struct sockaddr_in dest_addr;
    char packet[PACKET_SIZE];
    struct icmphdr *icmp_hdr;
    struct sockaddr_in r_addr;
    socklen_t addr_len = sizeof(r_addr);
    char recv_buffer[PACKET_SIZE];
    struct timeval send_time, recv_time;

    // 创建原始套接字
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置目标地址
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, destination_ip, &dest_addr.sin_addr) <= 0) {
        fprintf(stderr, "Error: Invalid IP address: %s\n", destination_ip);
        exit(EXIT_FAILURE);
    }

    // 开始发送ICMP包
    printf("PING %s: %d data bytes\n", destination_ip, PACKET_SIZE);
    while (infinite_mode || packets_sent < count) {
        // 构造ICMP包
        memset(packet, 0, PACKET_SIZE);
        icmp_hdr = (struct icmphdr *)packet;
        icmp_hdr->type = ICMP_ECHO;
        icmp_hdr->code = 0;
        icmp_hdr->un.echo.id = getpid();
        icmp_hdr->un.echo.sequence = packets_sent + 1;
        icmp_hdr->checksum = checksum(packet, sizeof(struct icmphdr));

        // 记录发送时间
        gettimeofday(&send_time, NULL);

        // 发送ICMP包
        if (sendto(sockfd, packet, sizeof(struct icmphdr), 0,
                   (struct sockaddr *)&dest_addr, sizeof(dest_addr)) <= 0) {
            perror("Sendto failed");
        } else {
            packets_sent++;
        }

        // 接收ICMP响应
        int bytes_received = recvfrom(sockfd, recv_buffer, PACKET_SIZE, 0,
                                      (struct sockaddr *)&r_addr, &addr_len);
        if (bytes_received > 0) {
            packets_received++;
            gettimeofday(&recv_time, NULL);

            // 提取IP头部和TTL
            struct iphdr *ip_hdr = (struct iphdr *)recv_buffer;
            int ip_header_len = ip_hdr->ihl * 4;
            int ttl = ip_hdr->ttl;

            // 计算RTT
            double rtt = time_diff(&send_time, &recv_time);

            printf("%d bytes from %s: icmp_seq=%d ttl=%d rtt=%.2f ms\n",
                   bytes_received - ip_header_len,
                   inet_ntoa(r_addr.sin_addr),
                   icmp_hdr->un.echo.sequence,
                   ttl,
                   rtt);
        } else {
            perror("Recvfrom failed");
        }

        sleep(1); // 延迟1秒
    }

    // 打印最终统计信息
    print_statistics();
    close(sockfd);
    return 0;
}
