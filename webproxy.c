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

/*
 *  Produces an error message in accordance with the error encountered and sends message to client
 */
void handle_error(int sock, int err_code, char *protocol, char *err_msg)
{
    puts("ERROR HANDLER");
    char err[MAXBUFSIZE];
    switch(err_code)
    {
        case 400:
            sprintf(err, "%s 400 Bad Request\r\n<html><body>400 Bad Request Reason: %s</body></html>\r\n\r\n", protocol, err_msg);
            break;

        case 501:
            sprintf(err, "%s 501 Not Implemented\r\n<html><body>501 Not Implemented Reason: %s</body></html>\r\n\r\n", protocol, err_msg);
            break;

        default:
            sprintf(err, "%s 500 Internal Server Error: cannot allocate memory\r\n\r\n", protocol);
            break;
    }
    send(sock, err, strlen(err), 0);
    return;
}

/*
 * Proxy has determined that the request is a valid GET request,
 * this function will process the request, send it to the requested address
 * and return results
 */
int process_get_request(int sock, char *uri, char *request, char *protocol)
{
    struct sockaddr_in req_addr;
    struct hostent *host;
    struct in_addr **host_addr;

    char *body;
    char full_req[MAXBUFSIZE];
    char response[MAXBUFSIZE];
    int host_sock;

    char *http_type = malloc(sizeof(char) * MAXBUFSIZE);
    char *full_path = malloc(sizeof(char) * MAXBUFSIZE);
    char *host_path = malloc(sizeof(char) * MAXBUFSIZE);
    char *file_path = malloc(sizeof(char) * MAXBUFSIZE);
    char *host_header = malloc(sizeof(char) * MAXBUFSIZE);
    char *port_no;

    memset(http_type, 0, strlen(http_type));
    memset(full_path, 0, strlen(full_path));
    memset(host_path, 0, strlen(host_path));
    memset(file_path, 0, strlen(file_path));
    memset(host_header, 0, strlen(host_header));


    /*Determine if the request has the chars http in front, if so discard so gethostbyname will work*/
    if(!strncmp(uri, "http://", strlen("http://")) || !strncmp(uri, "https://", strlen("https://")))
    {
        sscanf(uri, "%[^/]//%s", http_type, full_path);
        sscanf(full_path, "%[^/]/%s", host_path, file_path);
    }

    else
    {
        sscanf(uri, "%[^/]/%s", host_path, file_path);
    }

    /*Process port number if included */
    uint16_t i;
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

    /*Extract the body of the request*/
    char host_header_cpy[64];
    body = strdup(request);
    char *detritus = strsep(&body, "\n");
    printf("BODY BEFORE DISMEMBERMENT: %s\nstrlen(body): %d\n", body, (int) strlen(body));

    if(strlen(body) > 0)
    {
        char *body_dup = strdup(body);
        host_header = strsep(&body_dup, "\n");
        memset(host_header_cpy, 0, sizeof(host_header_cpy));
        strncpy(host_header_cpy, host_header, strlen(host_header)-1);
    }

    else
    {
        sprintf(host_header,"Host: %s", host_path);
        printf("host header: %s\n", host_header);
        memset(host_header_cpy, 0, sizeof(host_header_cpy));
        strncpy(host_header_cpy, host_header, strlen(host_header));
    }

    /* Configure the outbound socket */
    bzero(&req_addr,sizeof(struct sockaddr_in));               //zero the struct
    req_addr.sin_family = AF_INET;                 //address family
    !port_no ? (req_addr.sin_port = htons(80)) : (req_addr.sin_port = htons((uint16_t) atoi(port_no)));     //sets port to network byte order
    req_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*host_addr[0])); //sets IP address

    if((host_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        fprintf(stderr, "Unable to connect to host\n");
        return 1;
    }

    if(connect(host_sock, (struct sockaddr *)&req_addr, sizeof(req_addr)) < 0)
    {
        fprintf(stderr, "Unable to connect to host\n");
        return 1;
    }

    /*reassemble request*/

    sprintf(full_req, "GET /%s %s\r\n%s\r\nConnection: close\r\n\r\n", file_path, protocol, host_header_cpy);
    printf("-----------full request----------\n%s\n--------------------------------\n", full_req);
    send(host_sock, full_req, strlen(full_req), 0);

    int bytes_recv = 0;
    while((bytes_recv = recv(host_sock, response, MAXBUFSIZE, 0)) > 0)
    {
        send(sock, response, bytes_recv, 0);//possible problem here
        memset(response, 0, sizeof(response));
    }

    free(http_type);
    free(full_path);
    free(host_path);
    free(file_path);
    free(port_no);
    free(detritus);

    return 0;
}

/*
 * Generic request handler: figure out if it is a well-formed get request
 */
int handle_proxy(int *sp)
{
	char request[MAXBUFSIZE];
    char method[MAXBUFSIZE];
    char uri[MAXBUFSIZE];
    char protocol[MAXBUFSIZE];

	int sock = *sp;

	/* Parse request    */
    recv(sock, request, MAXBUFSIZE, 0);
    sscanf(request, "%s %s %s", method, uri, protocol);
    printf("in handle proxy, method: %s\nprotocol: %s\n", method, protocol);

    /*check to see if the protocol is supported*/
    if(strcmp(protocol, "HTTP/1.0"))
    {
        handle_error(sock, 400, protocol, "Invalid HTTP-Version");
        puts("error 501 Not implemented reason: protocol");
        close(sock);
        pthread_exit(NULL);
    }

    /*check to see if the method is defined in RFC1945*/
    if(strcmp(method, "GET") && strcmp(method, "HEAD") && strcmp(method, "POST") && strcmp(method, "PUT") && strcmp(method, "DELETE") && strcmp(method, "LINK") && strcmp(method, "UNLINK"))
    {
        handle_error(sock, 400, protocol, "Invalid Method");
        puts("INVALID METHOD TYPE 400 Bad Request");
        close(sock);
        pthread_exit(NULL);
    }

    /*check to see if the method is supported*/
    if(strcmp(method, "GET"))
    {
        handle_error(sock, 501, protocol, method);
        puts("METHOD UNSUPPORTED 501 NOT IMPLEMENTED");
        close(sock);
        pthread_exit(NULL);
    }

        /*must be get request, process*/
    else
    {
        process_get_request(sock, uri, request, protocol);
    }

    close(sock);
    pthread_exit(NULL);
}

/*
 * main - make connection with client, spawn threads for each connection
 */
int main(int argc, char *argv[])
{
	int sock;
	int sock_connection;
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
        pthread_detach(connection_thread);
	}
    return 0;
}
