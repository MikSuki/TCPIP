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

#define PORT 8080

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

#define COMMAND_SIZE 5

int recv_data_flag = 0;

int main(int argc, char *argv[])
{
	const int MAX_CLIENTS = 2;
	int opt = 1;
	int server_socket,
		priority_sockfd,
		normal_sockfd;
	int addrlen, new_sockfd, client_socket[MAX_CLIENTS], activity;
	int max_sockfd;
	struct sockaddr_in address;
	int terminate = 0;
	fd_set readfds;
	char buffer[512];


	// create server socket
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(1);
	}

	// let server can connect many clients
	const int ALLOW_MANY_CLIENTS = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &ALLOW_MANY_CLIENTS, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		exit(1);
	}

	// set address
	address.sin_family = AF_INET;
	// INADDR_ANY is used when you don't need to bind a socket to a specific IP.
	// When you use this value as the address when calling bind(),
	// the socket accepts connections to all the IPs of the machine.
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// bind server socket to address
	if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(1);
	}

	// set maximum length of client queue
	if (listen(server_socket, MAX_CLIENTS) < 0)
	{
		perror("listen");
		exit(1);
	}

	// accept the incoming connection
	addrlen = sizeof(address);
	puts("Waiting for connections ...");

	// wait for client connection
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if ((new_sockfd = accept(server_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
		{
			perror("accept");
			exit(1);
		}

		printf("New connection , socket fd is %d , ip is : %s , port : %d \n", new_sockfd, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

		client_socket[i] = new_sockfd;
	}

	// set two client socke fd
	if (client_socket[0] > client_socket[1])
	{
		priority_sockfd = client_socket[0];
		normal_sockfd = client_socket[1];
	}
	else
	{
		priority_sockfd = client_socket[1];
		normal_sockfd = client_socket[0];
	}

	// max sd
	max_sockfd = server_socket;
	if (priority_sockfd > max_sockfd)
		max_sockfd = priority_sockfd;

	while (!terminate)
	{
		FD_ZERO(&readfds);
		// add three socket to set
		FD_SET(server_socket, &readfds);
		FD_SET(priority_sockfd, &readfds);
		FD_SET(normal_sockfd, &readfds);

		// wait for an activity on one of the sockets
		activity = select(max_sockfd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR))
		{
			printf("select error");
		}
		// ------------------------------
		// priority client signal
		// ------------------------------
		if (FD_ISSET(priority_sockfd, &readfds))
		{
			int len = read(priority_sockfd, buffer, sizeof(buffer));
			char cmd[6];
			strncpy(cmd, &buffer[0], COMMAND_SIZE);

			if (len == 0)
			{
				terminate = 1;
			}
			// if (buffer[0] == 'p' && buffer[1] == '-')
			if (strcmp(cmd, "start") == 0)
			{
				recv_data_flag = 1;
			}
			else if (strcmp(cmd, "stop ") == 0)
			{
				recv_data_flag = 0;
			}
			printf(MAG"priority socket: "RED "%s\n", buffer);
			printf(RESET);
			// }
		}
		// ------------------------------
		// normal client signal
		// ------------------------------
		if (recv_data_flag && FD_ISSET(normal_sockfd, &readfds))
		{
			read(normal_sockfd, buffer, sizeof(buffer));
			printf("normal socket: "CYN "%s\n" RESET, buffer);
		}
	}

	close(server_socket);
	close(priority_sockfd);
	close(normal_sockfd);
	printf("disconnect~\n");

	return 0;
}
