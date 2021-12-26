#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define MAX 80
#define BUFFER_SIZE 32
#define PORT 8080
#define SA struct sockaddr

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

int sockfd;
int priority_sockfd;
char buff[MAX];
char cmd[BUFFER_SIZE];
int send_flag = 0;
int global_cnt = 0;

void sendToServer(int sockfd, char *message)
{
	char m[BUFFER_SIZE * 2];
	++global_cnt;
	sprintf(m, "%s %d", message, global_cnt);
	// strcat(message, itoa(global_cnt));
	write(sockfd, m, sizeof(m));
}

void send_garbage_message(void *message)
{
	int cnt = 0;
	while (1)
	{
		// if (++cnt > 1)
		// {
		// printf("send %s\n", message);
		sendToServer(sockfd, message);
		// write(sockfd, message, sizeof(message));
		// cnt = 0;
		// }
	}
}

void process_priority_cmd()
{
	while (1)
	{
		if (!send_flag)
			continue;
		printf(RED);
		if (cmd[0] == 'p' && cmd[1] == '-')
		{
			printf("priority command\n");
			// write(priority_sockfd, cmd, BUFFER_SIZE);
			sendToServer(priority_sockfd, cmd);
		}
		else if (cmd[0] == 'n' && cmd[1] == '-')
		{
			printf("normal command\n");
		}
		else
		{
			printf("command error\n");
			// int n = write(priority_sockfd, cmd, BUFFER_SIZE);
			// if (n < 0)
			// 	perror("ERROR reading from socket");
		}
		printf(RESET);
		printf("send\n");
		send_flag = 0;
	}
}

int main()
{
	struct sockaddr_in servaddr, cli;

	// socket create and varification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	priority_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	printf("pri: %d\n", priority_sockfd);
	printf("nor: %d\n", sockfd);

	if (sockfd == -1)
	{
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("140.117.168.64");
	// servaddr.sin_addr.s_addr = inet_addr("10.0.0.1");
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	// connect the client socket to server socket
	if (connect(priority_sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	int n;

	pthread_t tid;

	pthread_create(&tid, NULL, send_garbage_message, "n-11111111");

	pthread_create(&tid, NULL, process_priority_cmd, NULL);

	while (1)
	{
		printf("input cmd\n");
		scanf("%s", cmd);
		printf("your cmd: %s\n", cmd);
		send_flag = 1;
		
		// printf("in while\n");
		// process_priority_cmd();
		// bzero(buff, sizeof(buff));
		// printf("Enter the string : ");
		// n = 0;
		// while ((buff[n++] = getchar()) != '\n')
		// 	;
		// write(sockfd, buff, sizeof(buff));
		// bzero(buff, sizeof(buff));
		// read(sockfd, buff, sizeof(buff));
		// printf("From Server : %s", buff);
		// if ((strncmp(buff, "exit", 4)) == 0)
		// {
		// 	printf("Client Exit...\n");
		// 	break;
		// }
	}

	// close sockets
	pthread_exit(NULL);
	close(sockfd);
	close(priority_sockfd);
}
