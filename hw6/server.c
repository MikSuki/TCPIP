// Example code: A simple server side code, which echos back the received message.
// Handle multiple socket connections with select and fd_set on Linux
#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>	   //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define PORT 8080

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

int stop = 0;

int main(int argc, char *argv[])
{
	const int MAX_CLIENTS = 2;
	int opt = 1;
	int server_socket, addrlen, new_socket, client_socket[MAX_CLIENTS], activity, valread, sd;
	int max_sd;
	struct sockaddr_in address;

	char buffer[512]; // data buffer of 1K

	// set of socket descriptors
	fd_set readfds;

	// a message
	char *message = "ECHO Daemon v1.0 \r\n";

	// initial client socket
	for (int i = 0; i < MAX_CLIENTS; i++)
		client_socket[i] = 0;

	// create server socket
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// let server can connect many clients
	const int ALLOW_MANY_CLIENTS = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &ALLOW_MANY_CLIENTS, sizeof(opt)) < 0)
	// if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&ALLOW_MANY_CLIENTS, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
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
		exit(EXIT_FAILURE);
	}

	// set maximum length of client queue
	if (listen(server_socket, MAX_CLIENTS) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	// accept the incoming connection
	addrlen = sizeof(address);
	puts("Waiting for connections ...");

	while (1)
	{
		// clear the socket set
		FD_ZERO(&readfds);

		// add master socket to set
		FD_SET(server_socket, &readfds);
		max_sd = server_socket;

		// add child sockets to set
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			// socket descriptor
			sd = client_socket[i];

			// if valid socket descriptor then add to read list
			if (sd > 0)
				FD_SET(sd, &readfds);

			// highest file descriptor number, need it for the select function
			if (sd > max_sd)
				max_sd = sd;
		}

		// wait for an activity on one of the sockets , timeout is NULL ,
		// so wait indefinitely
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR))
		{
			printf("select error");
		}

		// If something happened on the master socket ,
		// then its an incoming connection
		if (FD_ISSET(server_socket, &readfds))
		{
			if ((new_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			// inform user of socket number - used in send and receive commands
			printf("New connection , socket fd is %d , ip is : %s , port : %d \n",
				   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

			// send new connection greeting message
			// if (send(new_socket, message, strlen(message), 0) != strlen(message))
			// {
			// 	perror("send");
			// }

			puts("Welcome message sent successfully");

			// add new socket to array of sockets
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				// if position is empty
				if (client_socket[i] == 0)
				{
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n", i);

					break;
				}
			}
		}

		// else its some IO operation on some other socket
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			sd = client_socket[i];

			if (FD_ISSET(sd, &readfds))
			{
				// Check if it was for closing , and also read the
				// incoming message
				// if ((valread = read(sd, buffer, sizeof(buffer))) == 0)
				// {
				// 	// Somebody disconnected , get his details and print
				// 	getpeername(sd, (struct sockaddr *)&address,
				// 				(socklen_t *)&addrlen);
				// 	printf("Host disconnected , ip %s , port %d \n",
				// 		   inet_ntoa(address.sin_addr), ntohs(address.sin_port));

				// 	// Close the socket and mark as 0 in list for reuse
				// 	close(sd);
				// 	client_socket[i] = 0;
				// }
				read(sd, buffer, sizeof(buffer));

				// Echo back the message that came in
				// else
				// {
				// printf("in \n");
				// strcpy(buffer, "i know~\n");
				// set the string terminating NULL byte on the end
				// of the data read
				// send(sd, buffer, strlen(buffer), 0);

				if (buffer[0] == 'p' && buffer[1] == '-')
				{
					// printf("priority command\n");
					printf(RED "%s\n", buffer);
					stop = 1;
				}
				else if (buffer[0] == 'n' && buffer[1] == '-')
				{
					// printf("normal command\n");
					if (!stop)
						printf(BLU "%s\n", buffer);
				}
				else
				{
					printf(YEL "command error~\n");
				}
				printf(RESET);
				// }
			}
		}
	}

	return 0;
}
