#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>


int TIMEOUT = 5000;

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

int is_time_out(struct timeval start, struct timeval end) {
    // printf("diff: %f", (end.tv_sec - start.tv_sec)*1000.0 + (end.tv_usec - start.tv_usec)/1000.0);
    return (end.tv_sec - start.tv_sec)*1000.0 + (end.tv_usec - start.tv_usec)/1000.0 > TIMEOUT;
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
    // set socket timeout
    struct timeval time_start, time_cur;
    time_start.tv_sec = TIMEOUT / 1000;
    time_start.tv_usec = 0;  
    setsockopt(sock_id, SOL_SOCKET, SO_RCVTIMEO, &time_start, sizeof(time_start));
    
    // buffer for request and reply
    char icmp_req_buffer[BUFFER_SIZE], 
        icmp_reply_buffer[BUFFER_SIZE]; 
    struct icmp *icmp_req = (struct icmp *) icmp_req_buffer;
    icmp_req->icmp_type = ICMP_ECHO;
    // htons() converts a u_short from host to TCP/IP network byte order (which is big-endian).
    icmp_req->icmp_code = htons(0);  
    icmp_req->icmp_id = htons(pid);    
    struct ip *reply = (struct ip *) icmp_reply_buffer;

    int flag_timeout = 0;

    //              hop distance
	for(ttl=1; ttl<=atoi(argv[1]); ttl++) {
        // set TTL
        setsockopt(sock_id, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)); 
        // set sequence number
        icmp_req->icmp_seq = htons(0);  
        // set check sum
        icmp_req->icmp_cksum = 0;
        icmp_req->icmp_cksum = checksum((uint16_t*) icmp_req, ICMP_HEADER_LEN);
        icmp_req_buffer[8] = 'm';
        icmp_req_buffer[9] = '1';
        icmp_req_buffer[10] = '0';
        icmp_req_buffer[11] = '3';
        icmp_req_buffer[12] = '0';
        icmp_req_buffer[13] = '4';
        icmp_req_buffer[14] = '0';
        icmp_req_buffer[15] = '0';
        icmp_req_buffer[16] = '5';
        // send packet
        sendto(sock_id, icmp_req_buffer, ICMP_HEADER_LEN+19, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        gettimeofday(&time_start, NULL);

        // wait fot correct ICMP reply
		while(1) {

            // recieve packet
			int bytes_recv = recvfrom(sock_id, icmp_reply_buffer, BUFFER_SIZE, 0, 0, 0);
            // set icmp header
            struct icmp *icmp_header = (struct icmp *) (icmp_reply_buffer + reply->ip_hl*4);  

            gettimeofday(&time_cur, NULL);
            if(is_time_out(time_start, time_cur)) {
                flag_timeout = 1;
                printf("TIMEOUT!\n");
                break;
            }

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

            // set reply
            reply = (struct ip *) icmp_reply_buffer;
            printf("%3d   %-16s\r\n", ttl, inet_ntoa(reply->ip_src));
			
            // already to destination
            if(icmp_header->icmp_type == ICMP_ECHOREPLY){
                ++terminate;
            }

            break;
		}

		if(terminate > 1 || flag_timeout) break;
    }

    if(terminate > 1)
        printf("hop distance > destination distance\r\n");
    return 0;
}
