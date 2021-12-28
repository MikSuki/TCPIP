#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 8080

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

#define BUFFER_SIZE 32
#define COMMAND_SIZE 5

int normal_sockfd;
int priority_sockfd;
char input[BUFFER_SIZE];
char data[512];
static int global_cnt = 0;
int send_data_flag = 0;
int terminate = 0;

void sendToServer(int sockfd, char *message)
{
	char m[BUFFER_SIZE * 2];
	++global_cnt;
	sprintf(m, "%s %s %d", message, "   ", global_cnt);
	write(sockfd, m, sizeof(m));
}

// normol socket: send data
void *send_data(void *message)
{
	while (1)
	{
		if (send_data_flag)
			sendToServer(normal_sockfd, message);
	}
}

// priority socket: send command
void send_cmd()
{
	char cmd[COMMAND_SIZE + 1];
	strncpy(cmd, &input[0], COMMAND_SIZE);

	if (strcmp(cmd, "start") == 0)
	{
		printf(GRN "start\n" RESET);
		send_data_flag = 1;
		sendToServer(priority_sockfd, cmd);
	}
	else if (strcmp(cmd, "stop") == 0)
	{
		printf(GRN "stop\n" RESET);
		send_data_flag = 0;
		sendToServer(priority_sockfd, cmd);
	}
	else if (strcmp(cmd, "quit") == 0)
	{
		printf(GRN "quit\n" RESET);
		sendToServer(priority_sockfd, cmd);
		terminate = 1;
	}
	else
	{
		printf(RED "command error~\n" RESET);
	}
	printf("cnt is: %d\n", global_cnt);
}

int main()
{
	struct sockaddr_in servaddr;

	normal_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	priority_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (priority_sockfd < normal_sockfd)
	{
		priority_sockfd ^= normal_sockfd;
		normal_sockfd ^= priority_sockfd;
		priority_sockfd ^= normal_sockfd;
	}

	if (normal_sockfd == -1)
	{
		printf("socket creation failed...\n");
		exit(0);
	}
	if (priority_sockfd == -1)
	{
		printf("socket creation failed...\n");
		exit(0);
	}

	// set address
	char str_ip[19];
	printf("\n----------------------------------");
	printf("\n  input " MAG "IP:\n" RESET);
	printf("----------------------------------\n\n");
	scanf("%s", str_ip);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(str_ip);
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(normal_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
	{
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf(CYN "normal socket" RESET ": connected to the server..\n");

	// connect the client socket to server socket
	if (connect(priority_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
	{
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf(CYN "priority socket" RESET ": connected to the server..\n");

	pthread_t tid;

	printf("\n----------------------------------");
	printf("\n  input " GRN "data:\n" RESET);
	printf("----------------------------------\n\n");
	scanf("%s", data);
	// data[0] = 'd';
	// data[1] = 'a';
	// data[2] = 't';
	// data[3] = 'a';

	pthread_create(&tid, NULL, send_data, data);

	while (!terminate)
	{
		printf("\n----------------------------------\n");
		printf("  input " YEL " cmd:\n" RESET);
		printf("----------------------------------\n\n");
		printf(RED);
		scanf("%s", input);
		printf(RESET);
		send_cmd();
	}

	pthread_cancel(tid);
	close(normal_sockfd);
	close(priority_sockfd);
}
