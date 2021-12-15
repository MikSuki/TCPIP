#include "pcap.h"
#include <sys/types.h>
#include <pcap/pcap.h>
#include <netinet/in.h>
#include <string.h>

extern pid_t pid;
extern u16 icmp_req;

static const char *dev = "eno1";
static char *net;
static char *mask;

static char filter_string[FILTER_STRING_SIZE] = "icmp";

static pcap_t *p;
static struct pcap_pkthdr hdr;

/*
 * This function is almost completed.
 * But you still need to edit the filter string.
 */
void pcap_init(const char *dst_ip, int timeout)
{
	int ret;
	char errbuf[PCAP_ERRBUF_SIZE];

	bpf_u_int32 netp;
	bpf_u_int32 maskp;

	struct in_addr addr;

	struct bpf_program fcode;

	struct pcap_pkthdr header; /* The header that pcap gives us */

	ret = pcap_lookupnet(dev, &netp, &maskp, errbuf);
	if (ret == -1)
	{
		fprintf(stderr, "%s\n", errbuf);
		exit(1);
	}

	addr.s_addr = netp;
	net = inet_ntoa(addr);
	if (net == NULL)
	{
		perror("inet_ntoa");
		exit(1);
	}

	addr.s_addr = maskp;
	mask = inet_ntoa(addr);
	if (mask == NULL)
	{
		perror("inet_ntoa");
		exit(1);
	}

	p = pcap_open_live(dev, 8000, 1, timeout, errbuf);
	if (!p)
	{
		fprintf(stderr, "%s\n", errbuf);
		exit(1);
	}

	/*
	 *    you should complete your filter string before pcap_compile
	 */

	if (pcap_compile(p, &fcode, filter_string, 0, maskp) == -1)
	{
		pcap_perror(p, "pcap_compile");
		exit(1);
	}

	if (pcap_setfilter(p, &fcode) == -1)
	{
		pcap_perror(p, "pcap_setfilter");
		exit(1);
	}

	int cnt = 1;
	while (1)
	// while (--cnt >= 0)
	{
		/* Grab a packet */
		const u_char *packet = pcap_next(p, &header);

		printf("Grabbed packet of length %d\n", header.len);
		printf("Ethernet address length is %d\n", ETHER_HDR_LEN);

		// const struct sniff_ethernet *ethernet; /* The ethernet header */
		// const struct sniff_ip *ip;			   /* The IP header */
		// const struct sniff_tcp *tcp;		   /* The TCP header */
		// const char *payload;				   /* Packet payload */
		// /* For readability, we'll make variables for the sizes of each of the structures */
		// int size_ethernet = sizeof(struct sniff_ethernet);
		// int size_ip = sizeof(struct sniff_ip);
		// int size_tcp = sizeof(struct sniff_tcp);

		// ip = (struct sniff_ip *)(packet + ether_header);
		// tcp = (struct sniff_tcp *)(packet + size_ethernet + size_ip);
		// payload = (u_char *)(packet + size_ethernet + size_ip + size_tcp);

		const struct sniff_ethernet *ethernet; /* The ethernet header */
		// const struct sniff_ip *ip;			   /* The IP header */
		const struct sniff_tcp *tcp; /* The TCP header */
		const char *payload;		 /* Packet payload */

		u_int size_ip;
		u_int size_tcp;

		// ethernet = (struct sniff_ethernet *)(packet);
		const struct sniff_ip *ip = (struct sniff_ip *)(packet + SIZE_ETHERNET);

		// printf("type: %d\n", ip->ip_tos);
		printf("protocol: %d\n", ip->ip_p);
		char str_src_ip[INET_ADDRSTRLEN];
		// inet_ntop(AF_INET, &ip->ip_src, str_src_ip, INET_ADDRSTRLEN);

		inet_ntop(AF_INET, &(ip->ip_src), str_src_ip, INET_ADDRSTRLEN);
		printf("src: %s\n", str_src_ip);
		inet_ntop(AF_INET, &(ip->ip_dst), str_src_ip, INET_ADDRSTRLEN);
		printf("dst: %s\n", str_src_ip);
		size_ip = IP_HL(ip) * 4;
		printf("size: %d\n", size_ip);

		const u_char *ptr = packet + SIZE_ETHERNET + size_ip;

		struct icmphdr *icmp_header = (struct icmphdr *)(packet + SIZE_ETHERNET + size_ip);

		printf("type: %d\n", ptr[0]);
		printf("seq: %d\n", ptr[6] + ptr[7]);
		// printf("type: %d\n", icmp_header->type);
		// printf("seq: %d\n", icmp_header->un.echo.sequence );

		// if (size_ip < 20)
		// {
		// 	printf("   * Invalid IP header length: %u bytes\n", size_ip);
		// 	return;
		// }

		// /* lets start with the ether header... */
		// struct ip_header *eptr = (struct ip_header *)packet;
		// printf("Ethernet type hex:%x dec:%d is an IP packet\n",
		// 	   ntohs(eptr->ip_type),
		// 	   ntohs(eptr->ip_type));

		// /* Do a couple of checks to see what packet type we have..*/
		// if (ntohs(eptr->ether_type) == ETHERTYPE_IP)
		// {
		// 	printf("Ethernet type hex:%x dec:%d is an IP packet\n",
		// 		   ntohs(eptr->ether_type),
		// 		   ntohs(eptr->ether_type));
		// }
		// else if (ntohs(eptr->ether_type) == ETHERTYPE_ARP)
		// {
		// 	printf("Ethernet type hex:%x dec:%d is an ARP packet\n",
		// 		   ntohs(eptr->ether_type),
		// 		   ntohs(eptr->ether_type));
		// }
		// else
		// {
		// 	printf("Ethernet type %x not IP", ntohs(eptr->ether_type));
		// 	exit(1);
		// }

		// /* And close the session */
		// pcap_close(p);
	}
}

// For ICMP echo reply, please check the following fields:
// 1. The source in IP header
// 2. ICMP type
// 3. The ID in ICMP message is matching what you have set.
// 4. The sequence number in ICMP message is the same as echo request
int pcap_get_reply(void)
{
	const u_char *ptr;

	ptr = pcap_next(p, &hdr);

	/*
	 * google "pcap_next" to get more information
	 * and check the packet that ptr pointed to.
	 */
	// https://www.tcpdump.org/pcap.html

	return 0;
}
