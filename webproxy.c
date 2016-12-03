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
#include <pthread.h>

#define MAXBUFSIZE 256

int handle_proxy(int *sock)
{
    char msg_in[MAXBUFSIZE];
    char msg_out[MAXBUFSIZE];
    sprintf(msg_out, "Connected to server, please enter a command\n");
    send(*sock, msg_out, strlen(msg_out), 0);
    recv(*sock, msg_in, MAXBUFSIZE, 0);
    msg_in[strlen(msg_in)-1] = '\0';
    printf("msg 0: %s\n", msg_in);
    close(*sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int sock;
	int sock_connection;                //This will be our socket
	struct sockaddr_in proxy_addr, client_addr;
	unsigned int client_addr_length = sizeof(struct sockaddr_in);         //length of the sockaddr_in structure

	if (argc != 2)
	{
		fprintf(stderr, "USAGE: ./webproxy <port no>\n");
		exit(1);
	}

	proxy_addr.sin_family = AF_INET;                   //address family
	proxy_addr.sin_port = htons((uint16_t) atoi(argv[1]));        	//htons() sets the port # to network byte order
	proxy_addr.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine

	//Build Socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("unable to create socket\n");
	}
	printf("socket initialized\n");

	
	//Bind Socket
	if (bind(sock, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) < 0)
	{
		fprintf(stderr, "unable to bind socket\n");
        exit(1);
	}
	printf("socket bind is complete\n");

	listen(sock, 3);

	printf("listening for connections\n");
	
	
	//Grab connection, spawn a new thread to handle it
	while(1)
	{	
		if((sock_connection = accept(sock, (struct sockaddr *)&client_addr, &client_addr_length)) > 0)
		{
            puts("connection formed");
            pthread_t connection_thread;
            int sock_dup = sock_connection;
            if(pthread_create(&connection_thread, NULL, (void * (*)(void *)) handle_proxy, (void *) &sock_dup))
            {
                perror("ERROR creating thread");
                exit(1);
            }
            puts("thread launched");
            pthread_join(connection_thread, NULL);

		}
	}
}