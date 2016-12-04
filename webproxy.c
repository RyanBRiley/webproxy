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
#include <netdb.h>

#define MAXBUFSIZE 256

int process_get_request(int sock, char *request)
{
    struct sockaddr_in req_addr;
    struct hostent *host;
    struct in_addr **host_addr;

    int host_sock;
    int connection;

    char *token = strsep(&request, " ");
    printf("in process get, token: %s\n", token);
    if(!strncmp(token, "http://", strlen("http://")) || !strncmp(token, "https://", strlen("https://")))
    {
        char *token_dup = strdup(token);
        strtok(token_dup, "://");
        host = gethostbyname(token+strlen(token_dup)+3);
    }
    else
    {
        host = gethostbyname(token);
    }
    host_addr = (struct in_addr **)host->h_addr_list;

    bzero(&req_addr,sizeof(struct sockaddr_in));               //zero the struct
    req_addr.sin_family = AF_INET;                 //address family
    req_addr.sin_port = htons(80);      //sets port to network byte order
    req_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*host_addr[0])); //sets server1 IP address

    if((host_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        fprintf(stderr, "Unable to connect to host\n");
        exit(1);
    }
    if((connection = connect(host_sock, (struct sockaddr *)&req_addr, sizeof(req_addr))) < 0)
    {
        fprintf(stderr, "Unable to connect to host\n");
        exit(1);
    }
    puts("PROCESS GET REQUEST");
    printf("request: %s\n", request);
    exit(1);
}

int handle_proxy(int *sp)
{
	char request[MAXBUFSIZE];
    char status[MAXBUFSIZE];
    char *token;
    char *request_dup;

	int sock = *sp;

	/* Parse request    */
    recv(sock, request, MAXBUFSIZE, 0);
    printf("request: %s\n", request);
    request_dup = strdup(request);
    token = strsep(&request_dup, " ");
    if(strcmp(token, "GET") && strcmp(token, "HEAD") && strcmp(token, "POST") && strcmp(token, "PUT") && strcmp(token, "DELETE") && strcmp(token, "LINK") && strcmp(token, "UNLINK"))
    {
        puts("INVALID METHOD TYPE");
        pthread_exit(NULL);
    }
    if(strcmp(token, "GET"))
    {
        puts("METHOD UNSUPPORTED");
        pthread_exit(NULL);
    }
    else
    {
        printf("request_dup: %s\n", request_dup);
        process_get_request(sock, request_dup);
    }
    printf("token: %s\n", token);
    close(sock);
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
	while((sock_connection = accept(sock, (struct sockaddr *)&client_addr, &client_addr_length)) > 0)
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
    return 0;
}
