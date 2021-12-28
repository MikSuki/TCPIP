#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


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



int main(int argc, char* argv[]) {
    
    if(argc != 3) { 
		printf("command error\r\n"); 
		exit(1); 
	}

    const int ICMP_HEADER_LEN = 8;
    const int BUFFER_SIZE = 1024;
    // get pid
    const int pid = getpid();  
    int ttl;
    // if terminate > 1, hop distance > destination distance
    int terminate = 0;  
    struct sockaddr_in dest_addr;
    bzero(&dest_addr, sizeof(dest_addr));
    // family = IPv4
    dest_addr.sin_family = AF_INET; 
	// convert IPv4 address to binary 
    inet_pton(AF_INET, argv[2], &dest_addr.sin_addr); 
	// socket file descriptor
    const int sock_id = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);  
    // buffer for request and reply
    char icmp_req_buffer[BUFFER_SIZE], 
        icmp_reply_buffer[BUFFER_SIZE]; 
    struct icmp *icmp_req = (struct icmp *) icmp_req_buffer;
    icmp_req->icmp_type = ICMP_ECHO;
    // htons() converts a u_short from host to TCP/IP network byte order (which is big-endian).
    icmp_req->icmp_code = htons(0);  
    icmp_req->icmp_id = htons(pid);    
    struct ip *reply = (struct ip *) icmp_reply_buffer;

    //              hop distance
	for(ttl=1; ttl<=atoi(argv[1]); ttl++) {
        // set TTL
        setsockopt(sock_id, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)); 
        // set sequence number
        icmp_req->icmp_seq = htons(0);  
        // set check sum
        icmp_req->icmp_cksum = 0;
        icmp_req->icmp_cksum = checksum((uint16_t*) icmp_req, ICMP_HEADER_LEN);
        // send packet
        sendto(sock_id, icmp_req_buffer, ICMP_HEADER_LEN, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
 
        // wait fot correct ICMP reply
		while(1) {
            // recieve packet
			int bytes_recv = recvfrom(sock_id, icmp_reply_buffer, BUFFER_SIZE, 0, 0, 0);
            // set icmp header
            struct icmp *icmp_header = (struct icmp *) (icmp_reply_buffer + reply->ip_hl*4);  

            // is number of bytes recieve less than 0?
			if(bytes_recv < 0) 
				continue;
			
            // is protocol ICMP?
			if(reply->ip_p != IPPROTO_ICMP) 
                continue;  

            // neither router's nor destination's reply
            if(icmp_header->icmp_type != ICMP_TIME_EXCEEDED && 
               icmp_header->icmp_type != ICMP_ECHOREPLY) 
                continue;

			// if packet is router's reply, 
            // shift header to origin request
			if(icmp_header->icmp_type == ICMP_TIME_EXCEEDED)
			    icmp_header = (struct icmp *) (icmp_header->icmp_data + ((struct ip *) (icmp_header->icmp_data))->ip_hl*4);
			
            // is this process pid?
			if(ntohs(icmp_header->icmp_id) != pid) 
                continue;

            printf("%-16s\r\n", inet_ntoa(reply->ip_src));

            // already to destination
            if(icmp_header->icmp_type == ICMP_ECHOREPLY){
                ++terminate;
                // set reply
			    reply = (struct ip *) icmp_reply_buffer;
                printf(RED"%-16s\r\n"RESET, inet_ntoa(reply->ip_src));
            }
            break;
		}

		if(terminate > 1) break;
    }

    if(terminate > 1)
        printf(RED"hop distance > destination distance\r\n"RESET);
    // else
    //     printf("%-16s\r\n", inet_ntoa(reply->ip_src));
    return 0;
}
