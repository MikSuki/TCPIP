#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include "util.h"
#include "fill_packet.h"
#include "pcap.h"

// ---------------------
// global var
// ---------------------
pid_t pid;
int seq = 1;
struct in_addr ip_src;

const int BUFFER_SIZE = 1024;

int main(int argc, char *argv[])
{
	int sockfd;
	int on = 1;

	pid = getpid();
	char *str_src_ip = get_ip();
	inet_pton(AF_INET, str_src_ip, &ip_src);

	/**
	 * set destination
	 */
	struct sockaddr_in dst;
	bzero(&dst, sizeof(dst));
	// family = IPv4
	dst.sin_family = AF_INET;
	// convert IPv4 address to binary
	inet_pton(AF_INET, "140.117.11.1", &dst.sin_addr);

	// -------------------
	// set ICMP request
	// -------------------

	struct myicmp *packet = (struct myicmp *)malloc(PACKET_SIZE);
	memset(packet, 0, sizeof(PACKET_SIZE));

	int count = DEFAULT_SEND_COUNT;
	int timeout = DEFAULT_TIMEOUT;

	// fill_iphdr(&(packet->ip_hdr), "140.117.168.63");
	fill_iphdr(&packet->ip_hdr, "140.117.11.1");
	fill_icmphdr(&packet->icmp_hdr);
	fill_cksum(&packet->icmp_hdr);

	char recv_buffer[BUFFER_SIZE];

	struct ip *receieve_packet = (struct ip *)recv_buffer;

	// fill_iphdr(&(packet->ip_hdr), "140.117.11.1");
	// fill_icmphdr(&(packet->icmp_hdr));
	// fill_cksum(&(packet->icmp_hdr));

	// /*
	//  * in pcap.c, initialize the pcap
	//  */
	// pcap_init("140.117.11.1", timeout);

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{
		perror("socket");
		exit(1);
	}

	if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
	{
		perror("setsockopt");
		exit(1);
	}

	// char icmp_req_buffer[BUFFER_SIZE];
	// struct icmp *icmp_req = (struct icmp *)icmp_req_buffer;
	// icmp_req->icmp_type = ICMP_ECHO;
	// // htons() converts a u_short from host to TCP/IP network byte order (which is big-endian).
	// icmp_req->icmp_code = htons(0);
	// icmp_req->icmp_id = htons(pid);
	// // set sequence number
	// icmp_req->icmp_seq = htons(500);
	// // set check sum
	// icmp_req->icmp_cksum = 0;
	// icmp_req->icmp_cksum = checksum((uint16_t *)icmp_req, sizeof(struct icmphdr));


	int ttl = 1;
	setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

	/*
	 *   Use "sendto" to send packets, and use "pcap_get_reply"(in pcap.c)
		 or use the standard socket like the one in the ARP homework
	 *   to get the "ICMP echo response" packets
	 *	 You should reset the timer every time before you send a packet.
	 */

	if (sendto(sockfd, packet, PACKET_SIZE, 0, (struct sockaddr *)&dst, sizeof(dst)) < 0)
	{
		perror("sendto");
		exit(1);
	}

	// sendto(sockfd, icmp_req, PACKET_SIZE, 0, (struct sockaddr *)&dst, sizeof(dst));

	/*
	 * in pcap.c, initialize the pcap
	 */
	// pcap_init("140.117.11.1", timeout);
	// pcap_get_reply();

	int bytes_recv = recvfrom(sockfd, recv_buffer, BUFFER_SIZE, 0, 0, 0);
	struct myicmp *reply = (struct myicmp *)recv_buffer;
	struct iphdr *ip_hdr = (struct iphdr *)recv_buffer;

	printf("tos: %d\n", ip_hdr->tos);
	printf("protocol: %d\n", ip_hdr->protocol);
	char str_ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(ip_hdr->saddr), str_ip, INET_ADDRSTRLEN);
	printf("src: %s\n", str_ip);
	inet_ntop(AF_INET, &(ip_hdr->daddr), str_ip, INET_ADDRSTRLEN);
	printf("dst: %s\n", str_ip);

	struct icmphdr *icmp_hdr = (struct icmphdr *)(recv_buffer + ip_hdr->ihl * 4);
	printf("type: %d\n", icmp_hdr->type);
	printf("code: %d\n", icmp_hdr->code);
	printf("id: %d\n", ntohs(icmp_hdr->un.echo.id));
	printf("seq: %d\n", ntohs(icmp_hdr->un.echo.sequence));

	return 0;

	while (1)
	{
		printf("recvfrom\n");
		// recieve packet
		int bytes_recv = recvfrom(sockfd, recv_buffer, BUFFER_SIZE, 0, 0, 0);

		receieve_packet = (struct ip *)recv_buffer;
		// is protocol ICMP?
		if (receieve_packet->ip_p != IPPROTO_ICMP)
			continue;
		// set icmp header
		struct icmp *icmp_header = (struct icmp *)(recv_buffer + receieve_packet->ip_hl * 4);

		printf("in\n");
		// gettimeofday(&time_cur, NULL);
		// if(is_time_out(time_start, time_cur)) {
		//     flag_timeout = 1;
		//     printf("TIMEOUT!\n");
		//     break;
		// }

		// is number of bytes recieve less than 0?
		if (bytes_recv < 0)
			continue;

		// neither router's nor destination's reply
		if (icmp_header->icmp_type != ICMP_TIME_EXCEEDED &&
			icmp_header->icmp_type != ICMP_ECHOREPLY)
			continue;

		// if packet is router's reply,
		// shift header to origin request
		if (icmp_header->icmp_type == ICMP_TIME_EXCEEDED)
			icmp_header = (struct icmp *)(icmp_header->icmp_data + ((struct ip *)(icmp_header->icmp_data))->ip_hl * 4);

		// // is this process pid?
		// if(ntohs(icmp_header->icmp_id) != pid)
		//     continue;

		// set reply
		receieve_packet = (struct ip *)recv_buffer;
		printf("%-16s\r\n", inet_ntoa(receieve_packet->ip_src));

		break;
	}

	free(packet);

	return 0;
}
