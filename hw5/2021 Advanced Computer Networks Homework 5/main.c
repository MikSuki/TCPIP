#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

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
	char* str_src_ip = get_ip();
	inet_pton(AF_INET, str_src_ip, &ip_src);

	/**
	 * set destination 
	 */
	struct sockaddr_in dst;
	bzero(&dst, sizeof(dst));
	// family = IPv4
	dst.sin_family = AF_INET;
	// convert IPv4 address to binary
	inet_pton(AF_INET, "140.117.168.42", &dst.sin_addr);

	// -------------------
	// set ICMP request
	// -------------------

	myicmp *packet = (myicmp *)malloc(PACKET_SIZE);
	memset(packet, 0, sizeof(PACKET_SIZE));
	int count = DEFAULT_SEND_COUNT;
	int timeout = DEFAULT_TIMEOUT;

	// fill_iphdr(&(packet->ip_hdr), "140.117.168.63");
	fill_iphdr(&packet->ip_hdr, "140.117.168.42");
	fill_icmphdr(&packet->icmp_hdr);
	fill_cksum(&packet->icmp_hdr);

	// fill_iphdr(&(packet->ip_hdr), "140.117.168.42");
	// fill_icmphdr(&(packet->icmp_hdr));
	// fill_cksum(&(packet->icmp_hdr));

	// /* 
	//  * in pcap.c, initialize the pcap
	//  */
	// pcap_init("140.117.168.42", timeout);

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
	{
		perror("socket");
		exit(1);
	}

	if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
	{
		perror("setsockopt");
		exit(1);
	}

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


	/* 
	 * in pcap.c, initialize the pcap
	 */
	pcap_init("140.117.168.42", timeout);

	free(packet);

	return 0;
}
