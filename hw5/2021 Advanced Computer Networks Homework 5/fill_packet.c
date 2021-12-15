#include "util.h"
#include "fill_packet.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>


uint16_t checksum(uint16_t *addr, int count) {
    int sum = 0;

    // main sum
    while(count > 1)  {
        sum += *addr++;
        count -= 2;
    }

    // add left-over bits
    if(count > 0) 
        sum += htons(*(uint8_t *)addr << 8);

    // 32-bits to 16-bits
    while(sum >> 16)
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
    // ICMP
    ip_hdr->ip_p = 0x01;
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
    icmp_hdr->type = 8;
    icmp_hdr->code = 0;
    icmp_hdr->un.echo.id = htons(pid);
    icmp_hdr->un.echo.sequence = htons(seq++);
}

u16 fill_cksum(struct icmphdr *icmp_hdr)
{
    //memset(&icmp_hdr->checksum, 0, sizeof(u16));
    // icmp_hdr->checksum = ip_checksum((u16 *)icmp_hdr, 64);
    uint16_t checkSum = checksum((uint16_t *)icmp_hdr, IP_HEADER_SIZE);
    icmp_hdr->checksum = 0;
    icmp_hdr->checksum = checkSum;
    return checkSum;
}

