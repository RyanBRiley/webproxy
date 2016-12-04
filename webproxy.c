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
//    printf("detritus: %s\nbody: %s\n-------------------\n", detritus, body);
//    printf("request before strktok: %s\n", request);
//    printf("METHOD: %s\nURI: %s\nPROTOCOL: %s\nBODY: %s\n", method, uri, protocol, body);
//    strcat(uri, "+{"); //Append uri with an escape character so strtok can find the end of the uri
//    printf("uri: %s\n", uri);
    char *uri_dup;
    char *cut_host;
    char *cut_uri;

    char *http_type = malloc(sizeof(char) * MAXBUFSIZE);
    char *full_path = malloc(sizeof(char) * MAXBUFSIZE);
    char *host_path = malloc(sizeof(char) * MAXBUFSIZE);
    char *file_path = malloc(sizeof(char) * MAXBUFSIZE);
    char *port_no;
    //memset(file_path, 0, sizeof(file_path));

    //sscanf(uri, "http://%s%[^/]", http_type,full_path);


//    sscanf(http_type, "%s/%s", full_path, host_path);


    if(!strncmp(uri, "http://", strlen("http://")) || !strncmp(uri, "https://", strlen("https://")))
    {
//        puts("condition 1");
        sscanf(uri, "%[^/]//%s", http_type, full_path);
//        printf("http_type: %s\n", http_type);
        sscanf(full_path, "%[^/]/%s", host_path, file_path);
//        printf("full_path: %s\n", full_path);
//        printf("host_path: %s\n", host_path);
//        printf("file_path: %s\n", file_path);
    }
    else
    {
//        puts("condition 2");
        sscanf(uri, "%[^/]/%s", host_path, file_path);
//        printf("full_path: %s\n", full_path);
//        printf("host_path: %s\n", host_path);
//        printf("file_path: %s\n", file_path);
    }

    int i;
    for(i = 0; i < strlen(host_path); i++)
    {
        if(host_path[i] == ':')
        {
            port_no = malloc(sizeof(char)*MAXBUFSIZE);
            strcpy(port_no, host_path+(i+1));
//            printf("port_no0000: %s\n", port_no);

            host_path[i] = '\0';
//            printf("host_path after removing port_no: %s\n", host_path);
            break;
        }
    }
/*    if(!port_no)
    {
        puts("port_no NULL");
    }
    else
    {
        puts("por_no NOT NULL");
    }*/

   /* if(file_path  == NULL)
    {
        puts("file path null!!!!!!!1");
    }
    else
    {
        puts("file path not null!!!!");
    }*/
    /*if(!strncmp(uri, "http://", strlen("http://")))
    {
        puts("condition 1");
        uri_dup = strdup(uri + strlen("http://"));
        cut_host = strtok(uri_dup, "/");
        printf("condition 1 cut host: %s\ncondition 1 uri_dup: %s\n", cut_host, uri_dup);
        cut_uri = strtok(NULL, "+{");
        printf("condition 1 cut uri: %s\ncondition 1 uri_dup: %s\n", cut_uri, uri_dup);
    }
    else if(!strncmp(uri, "https://", strlen("https://")))
    {
        puts("condition 2");

        uri_dup = strdup(uri + strlen("https://"));
        cut_host = strtok(uri_dup, "/");
        printf("condition 1 cut host: %s\ncondition 1 uri_dup: %s\n", cut_host, uri_dup);
        cut_uri = strtok(NULL, "+{");
        printf("condition 1 cut uri: %s\ncondition 1 uri_dup: %s\n", cut_uri, uri_dup);
    }
    else
    {
        puts("condition 3");
        uri_dup = strdup(uri);
        cut_host = strtok(uri_dup, "/");
        printf("condition 1 cut host: %s\ncondition 1 uri_dup: %s\n", cut_host, uri_dup);
        cut_uri = strtok(NULL, "+{");
        printf("condition 1 cut uri: %s\ncondition 1 uri_dup: %s\n", cut_uri, uri_dup);
    }*/
    host = gethostbyname(host_path);
/*    printf("uri_dup: %s\n", uri_dup);
    char *cut_uri_1 = strtok(uri_dup, "//");
    printf("cut_uri_1: %s\n", cut_uri_1);
    printf("uri_dup0: %s\n", uri_dup);
    char *cut_uri_2= strtok(NULL, "/");
    printf("cut_uri_2: %s\n", cut_uri_2);
    char *cut_uri_3 = strtok(NULL, "\r");
    printf("cut_rui_3: %s\n", cut_uri_3);
//    printf("request after strtok: %s\n", request);
    if(cut_uri_2 == NULL)
    {
        host = gethostbyname(cut_uri_1);
    }
    else
    {
        host = gethostbyname(cut_uri_2);
    }*/
    /*char *token = strsep(&request, " ");
    printf("in process get, token: %s\n", token);
    if(!strncmp(token, "http://", strlen("http://")) || !strncmp(token, "https://", strlen("https://")))
    {
        printf("TOKEN : %s\n", token);
        char *token_dup = strdup(token);
        strtok(token_dup, "://");
        printf("token_dup 00000: %s\n", token+strlen(token_dup)+3);
        char tokcpy[MAXBUFSIZE];
        strcpy(tokcpy, token+strlen(token_dup)+3);
        tokcpy[strlen(tokcpy) -1] = '\0';
        printf("tokcpy: %s\n",tokcpy);

        host = gethostbyname(tokcpy);


    }
    else
    {
        host = gethostbyname(token);
    }*/
//    printf("host name: %s\n", host->h_name);
    host_addr = (struct in_addr **)host->h_addr_list;

    bzero(&req_addr,sizeof(struct sockaddr_in));               //zero the struct
    req_addr.sin_family = AF_INET;                 //address family
    !port_no ? (req_addr.sin_port = htons(80)) : (req_addr.sin_port = htons((uint16_t) atoi(port_no)));     //sets port to network byte order
    req_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*host_addr[0])); //sets server1 IP address
//    printf("%s ", inet_ntoa(*host_addr[0]));
//    printf("req_addr: %s\n", inet_ntoa(inet_addr(req_addr.sin_addr.s_addr)));
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
    sprintf(full_req, "GET /%s %s \r\n%s\r\n\r\n", file_path, protocol, body);
//    !file_path ? sprintf(full_req, "GET / HTTP/1.0\r\n\r\n") : sprintf(full_req, "GET /%s HTTP/1.0\r\n\r\n", file_path);
    printf("full request: %s\n", full_req);
    send(host_sock, full_req, strlen(full_req), 0);
//    printf("bsent: %d\n", bsent);

    //recv(host_sock, response, MAXBUFSIZE, 0);
    //send(sock, response, MAXBUFSIZE, 0);
    while(recv(host_sock, response, 256, 0) > 0)
    {
        send(sock, response, 256, 0);
        printf("---------response---------\n%s\n------------------\n", response);
//        memset(response, 0, sizeof(response));

//        printf("bsent: %d\n",bsent);
    }
//    int brecv = (int) recv(host_sock, response, MAXBUFSIZE, 0);
//    printf("brecv: %d\n", brecv);
//    puts("PROCESS GET REQUEST");
//    printf("request: %s\n", request);
    pthread_exit(NULL);
}

int handle_proxy(int *sp)
{
	char request[MAXBUFSIZE];
//    char status[MAXBUFSIZE];
    char *token;
    char *request_dup;

	int sock = *sp;

	/* Parse request    */
    recv(sock, request, MAXBUFSIZE, 0);
//    printf("request: %s\n", request);
    request_dup = strdup(request);
    token = strsep(&request_dup, " ");
    printf("METHOD TYPE: %s\n", token);
//    printf("\n\n---Request---\n%s\n-------------\n", request);
    if(strcmp(token, "GET") && strcmp(token, "HEAD") && strcmp(token, "POST") && strcmp(token, "PUT") && strcmp(token, "DELETE") && strcmp(token, "LINK") && strcmp(token, "UNLINK") && strcmp(token, "CONNECT"))
    {
        puts("INVALID METHOD TYPE");
        pthread_exit(NULL);
    }
    if(strcmp(token, "GET") && strcmp(token, "CONNECT"))
    {
        puts("METHOD UNSUPPORTED");
        pthread_exit(NULL);
    }
    else
    {
//        printf("request_dup: %s\n", request_dup);
        process_get_request(sock, request);
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
