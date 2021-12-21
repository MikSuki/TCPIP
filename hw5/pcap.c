#include "pcap.h"
#include <sys/types.h>
#include <pcap/pcap.h>
#include <netinet/in.h>
#include <string.h>

extern pid_t pid;
extern u16 icmp_req;

static const char *dev = "enp2s0f5";
static char *net;
static char *mask;

// static char filter_string[FILTER_STRING_SIZE] = "icmp";
static char filter_string[FILTER_STRING_SIZE] = "icmp and src host 140.117.168.45 and dst host 140.117.168.64";

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

}

// For ICMP echo reply, please check the following fields:
// 1. The source in IP header
// 2. ICMP type
// 3. The ID in ICMP message is matching what you have set.
// 4. The sequence number in ICMP message is the same as echo request
int pcap_get_reply(void)
{
	// const u_char *ptr;

	// ptr = pcap_next(p, &hdr);

	/*
	 * google "pcap_next" to get more information
	 * and check the packet that ptr pointed to.
	 */
	// https://www.tcpdump.org/pcap.html


	int cnt = 1;
	while (1)
	// while (--cnt >= 0)
	{
		printf("Grabbed packet of length \n");
		/* Grab a packet */
		const u_char *ptr = pcap_next(p, &hdr);

		printf("Grabbed packet of length %d\n", hdr.len);
		printf("Ethernet address length is %d\n", ETHER_HDR_LEN);

		u_int size_ip;

		const struct sniff_ip *ip = (struct sniff_ip *)(ptr + SIZE_ETHERNET);

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

		struct icmphdr *icmp_header = (struct icmphdr *)(ptr + SIZE_ETHERNET + size_ip);

		printf("type: %d\n", icmp_header->type);
		printf("id: %d\n", ntohs(icmp_header->un.echo.id));
		printf("seq: %d\n", ntohs(icmp_header->un.echo.sequence));

	}

	return 0;
}
