#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

typedef char u8;
typedef unsigned short u16;

#define PACKET_SIZE 92
#define IP_HEADER_SIZE 8
#define IP_OPTION_SIZE 8
#define DEFAULT_SEND_COUNT 4
#define DEFAULT_TIMEOUT 1500
#define ICMP_HEADER_SIZE 8

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

int TIMEOUT = 1000;
char *interface;
uint16_t pid;
int seq = 0;
struct in_addr ip_src;

struct myicmp
{
    struct ip ip_hdr;
    u8 ip_option[8];
    struct icmphdr icmp_hdr;
    u8 data[0];
};

uint16_t checksum(uint16_t *addr, int count)
{
    int sum = 0;

    // main sum
    while (count > 1)
    {
        sum += *addr++;
        count -= 2;
    }

    // add left-over bits
    if (count > 0)
        sum += htons(*(uint8_t *)addr << 8);

    // 32-bits to 16-bits
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

// Fill the IP header according to the following format:
// 1. Header length = calculate by youself
// 2. Total length = calculate by youself
// 3. Id = 0
// 4. Flag = don’t fragment
// 5. TTL = 1
// 6. Protocol = ICMP
// 7. Checksum (You can let OS do it.)
void fill_iphdr(struct ip *ip_hdr, const char *dst_ip)
{
    // ipv4
    ip_hdr->ip_v = 4;
    ip_hdr->ip_hl = (uint8_t)IP_HEADER_SIZE;
    ip_hdr->ip_tos = 8;
    ip_hdr->ip_len = htons(PACKET_SIZE);
    ip_hdr->ip_id = 0;
    // don’t fragment
    ip_hdr->ip_off = htons(IP_DF);
    // TTL = 1
    ip_hdr->ip_ttl = 1;
    // ip_hdr->ip_ttl = 100;
    // ICMP
    ip_hdr->ip_p = 1;
    ip_hdr->ip_src = ip_src;
    inet_pton(AF_INET, dst_ip, &ip_hdr->ip_dst);
}

// Fill the ICMP packet according to the following format:
// 1. Checksum (You can let OS do it.)
// 2. ID: process id
// 3. Sequence number: Starting from 1, increase one by one.
// 4. Data : Your Student ID. Note that data size must correspond to IP header.

void fill_icmphdr(struct icmphdr *icmp_hdr)
{
    printf("pid: %d\n", pid);
    printf("seq: %d\n", seq);
    icmp_hdr->type = 8;
    icmp_hdr->code = 0;
    icmp_hdr->un.echo.id = htons(pid);
    icmp_hdr->un.echo.sequence = htons(seq);
    // icmp_hdr->un.echo.sequence = htons(500);
}

u16 fill_cksum(struct icmphdr *icmp_hdr)
{
    uint16_t checkSum = checksum((uint16_t *)icmp_hdr, 8);
    icmp_hdr->checksum = 0;
    icmp_hdr->checksum = checkSum;
    return checkSum;
}

double time_diff(struct timeval start, struct timeval end)
{
    return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
}

int is_time_out(struct timeval start, struct timeval end)
{
    return time_diff(start, end) > TIMEOUT;
}

char *get_my_ip()
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

char *get_my_netmask()
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);

    ioctl(fd, SIOCGIFNETMASK, &ifr);

    close(fd);

    return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

void remove_spaces(char *s)
{
    char *d = s;
    do
    {
        while (*d == ' ')
        {
            ++d;
        }
    } while (*s++ = *d++);
}

int main(int argc, char *argv[])
{
    // command error detect
    if (argc != 5 ||
        strcmp(argv[1], "-i") != 0 ||
        strcmp(argv[3], "-t") != 0)
    {
        printf("command error!\n");
        printf("sudo ./ipscanner -i INTERFACE -t TIMEOUT\n");
        exit(1);
    }

    pid = getpid();
    int sockfd;
    int on = 1;
    const int ttl = 1;

    // set interface
    interface = argv[2];

    // set timeout (ms)
    TIMEOUT = atoi(argv[4]) * 100;

    // get ip and mask
    char ip[16], netmask[16];
    strcpy(ip, get_my_ip());
    strcpy(netmask, get_my_netmask());

    // set ip_src
    bzero(&ip_src, sizeof(ip_src));
    inet_pton(AF_INET, ip, &ip_src);

    // buffer for request and reply
    char icmp_req_buffer[PACKET_SIZE],
        icmp_reply_buffer[PACKET_SIZE];

    struct ip *reply = (struct ip *)icmp_reply_buffer;
    struct sockaddr_in dst;
    struct timeval time_start, time_cur;

    // -------------------------
    //   scan sunbnet
    // -------------------------
    printf("my ip:        "YEL"%s\n"RESET, ip);
    printf("my netmask:   "YEL"%s\n"RESET, netmask);
    printf("my interface: "YEL"%s\n"RESET, interface);
    printf("my pid:       "YEL"%d\n"RESET, pid);
    printf("TIMEOUT:      "YEL"%dms\n"RESET, TIMEOUT);
    printf("\n----------------------\n"BLU"  start scan"RESET"\n----------------------\n\n");

    struct in_addr ipaddress, subnetmask;
    inet_pton(AF_INET, ip, &ipaddress);
    inet_pton(AF_INET, netmask, &subnetmask);

    unsigned long first_ip = ntohl(ipaddress.s_addr & subnetmask.s_addr) + 1;
    unsigned long last_ip = ntohl(ipaddress.s_addr | ~(subnetmask.s_addr)) - 1;

    for (unsigned long i = first_ip; i <= last_ip; ++i)
    {
        unsigned long theip = htonl(i);
        struct in_addr cur_ip = {theip};
        char *str_cur_ip = inet_ntoa(cur_ip);
        if (strcmp(ip, str_cur_ip) == 0)
            continue;
        printf("PING %s (data size = 10, id = %d, seq = %d, timeout = %dms)\n", str_cur_ip, pid, seq + 1, TIMEOUT);

        if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        {
            perror("socket");
            exit(1);
        }

        // if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
        // {
        //     perror("setsockopt");
        //     exit(1);
        // }

        bzero(&dst, sizeof(dst));
        // family = IPv4
        dst.sin_family = AF_INET;
        // convert IPv4 address to binary
        inet_pton(AF_INET, str_cur_ip, &dst.sin_addr);
        // set socket timeout
        time_start.tv_sec = 0;
        time_start.tv_usec = TIMEOUT * 1000;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &time_start, sizeof(time_start));
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &time_start, sizeof(time_start));

        struct icmp *packet = (struct icmp *)icmp_req_buffer;
        // ECHO REQUEST
        packet->icmp_type = 8;
        // htons() converts a u_short from host to TCP/IP network byte order (which is big-endian).
        packet->icmp_code = htons(0);
        packet->icmp_id = htons(pid);
        // set sequence number
        packet->icmp_seq = htons(++seq);

        // set check sum
        packet->icmp_cksum = 0;
        packet->icmp_cksum = checksum((uint16_t *)packet, PACKET_SIZE);
        

        // set student ID
        icmp_req_buffer[8] = 'm';
        icmp_req_buffer[9] = '1';
        icmp_req_buffer[10] = '0';
        icmp_req_buffer[11] = '3';
        icmp_req_buffer[12] = '0';
        icmp_req_buffer[13] = '4';
        icmp_req_buffer[14] = '0';
        icmp_req_buffer[15] = '0';
        icmp_req_buffer[16] = '5';
        icmp_req_buffer[17] = '6';

        setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
        gettimeofday(&time_start, NULL);

        if (sendto(sockfd, packet, PACKET_SIZE, 0, (struct sockaddr *)&dst, sizeof(dst)) < 0)
        {
            perror("sendto");
            exit(1);
        }

        // wait ICMP echo reply
        while (1)
        {
            // recieve packet
            int bytes_recv = recvfrom(sockfd, icmp_reply_buffer, PACKET_SIZE, 0, 0, 0);
            // set icmp header
            struct icmp *icmp_header = (struct icmp *)(icmp_reply_buffer + reply->ip_hl * 4);

            gettimeofday(&time_cur, NULL);
            if (is_time_out(time_start, time_cur))
            {
                printf("         Destination unreachable\n");
                break;
            }
            // is number of bytes recieve less than 0?
            if (bytes_recv < 0)
                continue;

            // is protocol ICMP?
            if (reply->ip_p != 1)
                continue;

            // is ECHO REPLY?
            if (icmp_header->icmp_type != 0)
                continue;

            // // if packet is router's reply,
            // // shift header to origin request
            // if (icmp_header->icmp_type == ICMP_TIME_EXCEEDED)
            //     icmp_header = (struct icmp *)(icmp_header->icmp_data + ((struct ip *)(icmp_header->icmp_data))->ip_hl * 4);

            // is this process pid?
            if (ntohs(icmp_header->icmp_id) != pid)
                continue;

            if (ntohs(icmp_header->icmp_seq) != seq)
                continue;

            // set reply
            reply = (struct ip *)icmp_reply_buffer;
            printf(RED"         Reply from : %-16s, ", inet_ntoa(reply->ip_src));
            printf("time : %fms\n"RESET, time_diff(time_start, time_cur));

            break;
        }
    }
}