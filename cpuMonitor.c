#include <stdio.h> // printf, sprintf
#include <stdlib.h> // exit, atoi, malloc, free
#include <time.h> // nanosleep
#include <string.h> // strcpy
#include <unistd.h> // read, write, close
#include <string.h> // memcpy, memset
#include <sys/socket.h> // socket, connect
#include <netinet/in.h> // struct sockaddr_in, struct sockaddr
#include <netdb.h> // struct hostent, gethostbyname

#define milSec 500

void error(const char *msg) {
        perror(msg); exit(0);
}

void request(const char *body)
{
    int i;

    /* first where are we going to send it? */
    int port = 80;
    char *host = "localhost";

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total, message_size;
    char *message, response[4096];

    /* How big is the message? */
    message_size=0;
    message_size+=strlen("POST / HTTP/1.0\r\n");
    message_size+=strlen("Content-Type: application/json");
    message_size+=strlen("\r\n");
    message_size+=strlen("Content-Length: \r\n")+strlen(body);
    message_size+=strlen("\r\n");
    message_size+=strlen(body);


    /* allocate space for the message */
    message=malloc(message_size);

    sprintf(message,"POST / HTTP/1.0\r\n");
    strcat(message,"Content-Type: application/json");
    strcat(message,"\r\n");
    sprintf(message+strlen(message), "Content-Length: %ld\r\n", strlen(body));
    strcat(message,"\r\n");

    strcat(message,body);


    /* What are we going to send? */
    printf("Request:\n%s\n",message);

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) error("ERROR, no such host");

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
            error("ERROR connecting");

    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
            bytes = write(sockfd,message+sent,total-sent);
            if (bytes < 0)
                    error("ERROR writing message to socket");
            if (bytes == 0)
                    break;
            sent+=bytes;
    } while (sent < total);

    /* receive the response */
    memset(response,0,sizeof(response));
    total = sizeof(response)-1;
    received = 0;
    do {
            bytes = read(sockfd,response+received,total-received);
            if (bytes < 0)
                    error("ERROR reading response from socket");
            if (bytes == 0)
                    break;
            received+=bytes;
    } while (received < total);

    if (received == total)
            error("ERROR storing complete response from socket");

    /* close the socket */
    close(sockfd);

    /* process response */
    printf("Response:\n%s\n",response);

    free(message);
}

int main()
{
        char body[256];
        char loadAvgString[256];
        long double a[4], b[4], loadavg;
        FILE *fp;

        struct timespec ts;
        ts.tv_sec = milSec / 1000;
        ts.tv_nsec = (milSec % 1000) * 1000000;

        for(;;)
        {
                fp = fopen("/proc/stat","r");
                fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
                fclose(fp);
                nanosleep(&ts, NULL);

                fp = fopen("/proc/stat","r");
                fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
                fclose(fp);

                loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3])) * 100;
                sprintf(loadAvgString, "%.2Lf", loadavg);

                strcpy(body,"{\"cpuUsage\":");
                strcat(body,loadAvgString);
                strcat(body,"}");

                request(body);
        }
}
