/******************************
Ryan Riley
CSCI 4273 - Network Systems
PA-4 Web Proxy
*******************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXBUFSIZE 256

int main(int argc, char *argv[])
{
	int sock;
	int sock_connection;                //This will be our socket
	int *sock_cp;
	struct sockaddr_in sin, remote;  
	unsigned int remote_length = sizeof(struct sockaddr_in);         //length of the sockaddr_in structure

	int config = configure_server();

	sin.sin_family = AF_INET;                   //address family
	sin.sin_port = htons(listenport);        	//htons() sets the port # to network byte order
	sin.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine

	//Build Socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("unable to create socket\n");
	}
	printf("socket initialized\n");

	
	//Bind Socket
	if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("unable to bind socket\n");
	}
	printf("socket bind is complete\n");

	listen(sock, 3);

	printf("listening for connections\n");
	
	
	//Grab connection, fork a new process to handle it
	while(1)
	{	
		if((sock_connection = accept(sock, (struct sockaddr *)&remote, (socklen_t *)&remote_length)) > 0)
		{	
			printf("connection  formed\n");	
			int pid = fork();
			if(pid == 0)
			{
				server_function(sock_connection);
				close(sock_connection);
				exit(0);

			}
			
		}
	}
	close(sock);
	return 0;
}