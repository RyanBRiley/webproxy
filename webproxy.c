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

#define MAXBUFSIZE 2048

int process_get_request(int sock, char *request)
{
    struct sockaddr_in req_addr;
    struct hostent *host;
    struct in_addr **host_addr;

    char method[MAXBUFSIZE];
    char uri[MAXBUFSIZE];
    char protocol[MAXBUFSIZE];
    char *body;
    char full_req[MAXBUFSIZE];
    char response[MAXBUFSIZE];
    int host_sock;

    sscanf(request, "%s %s %s", method, uri, protocol);
    body = strdup(request);
    char *detritus = strsep(&body, "\n");


    char *http_type = malloc(sizeof(char) * MAXBUFSIZE);
    char *full_path = malloc(sizeof(char) * MAXBUFSIZE);
    char *host_path = malloc(sizeof(char) * MAXBUFSIZE);
    char *file_path = malloc(sizeof(char) * MAXBUFSIZE);
    char *port_no;



    if(!strncmp(uri, "http://", strlen("http://")) || !strncmp(uri, "https://", strlen("https://")))
    {
        sscanf(uri, "%[^/]//%s", http_type, full_path);
        sscanf(full_path, "%[^/]/%s", host_path, file_path);
    }

    else
    {
        sscanf(uri, "%[^/]/%s", host_path, file_path);
    }

    int i;
    for(i = 0; i < strlen(host_path); i++)
    {
        if(host_path[i] == ':')
        {
            port_no = malloc(sizeof(char)*MAXBUFSIZE);
            strcpy(port_no, host_path+(i+1));

            host_path[i] = '\0';
            break;
        }
    }

    host = gethostbyname(host_path);

    host_addr = (struct in_addr **)host->h_addr_list;

    bzero(&req_addr,sizeof(struct sockaddr_in));               //zero the struct
    req_addr.sin_family = AF_INET;                 //address family
    !port_no ? (req_addr.sin_port = htons(80)) : (req_addr.sin_port = htons((uint16_t) atoi(port_no)));     //sets port to network byte order
    req_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*host_addr[0])); //sets server1 IP address

    if((host_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        fprintf(stderr, "Unable to connect to host\n");
        exit(1);
    }
    if(connect(host_sock, (struct sockaddr *)&req_addr, sizeof(req_addr)) < 0)
    {
        fprintf(stderr, "Unable to connect to host\n");
        exit(1);
    }

    sprintf(full_req, "GET /%s HTTP/1.0 \r\n%s\r\n\r\n", file_path, body);

    printf("full request: %s\n", full_req);

    send(host_sock, full_req, strlen(full_req), 0);

    while(recv(host_sock, response, 256, 0) > 0)
    {
        send(sock, response, 256, 0);
//        printf("---------response---------\n%s\n------------------\n", response);
        memset(response, 0, sizeof(response));
    }
    pthread_exit(NULL);
}

int handle_proxy(int *sp)
{
	char request[MAXBUFSIZE];
    char *token;
    char *request_dup;

	int sock = *sp;

	/* Parse request    */
    recv(sock, request, MAXBUFSIZE, 0);

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

        process_get_request(sock, request);
    }

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
//		pthread_join(connection_thread, NULL);
        pthread_detach(connection_thread);
	}
    return 0;
}
