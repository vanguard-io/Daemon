#include <stdio.h> // printf, sprintf
#include <stdlib.h> // exit, atoi, malloc, free
#include <time.h> // nanosleep
#include <unistd.h> // read, write, close
#include <string.h> // memcpy, memset, strcpy
#include <sys/socket.h> // socket, connect
#include <netinet/in.h> // struct sockaddr_in, struct sockaddr
#include <netdb.h> // struct hostent, gethostbyname
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>

#define milSec 500

void Log(const char *message, const int error)
{
        FILE *file;
        char filePath[1000];

        if(!error) {
                printf("%s\n", "not an error");
                strcpy(filePath, "/var/log/daemon_log");
        } else {
                strcpy(filePath, "/var/log/daemon_error");
        }

        file = fopen(filePath, "ab+");

        if (file == NULL) {
                // Unable to open file hence exit
                printf("\nUnable to open '%s' file.\n", filePath);
                printf("Please check whether file exists and you have write privilege.\n");
                exit(EXIT_FAILURE);
        }

        fputs(message, file);

        fclose(file);

        if(error) {
                printf("%s\n", "We broke");
                exit(1);
        }

}

void request(const char *body)
{
        int i;

        // first where are we going to send it?
        int port = 80;
        char host[1000] = "localhost";
        char error_message[1000] = "";

        struct hostent *server;
        struct sockaddr_in serv_addr;
        int sockfd, bytes, sent, received, total, message_size;
        char *message, response[4096];

        // How big is the message?
        message_size=0;
        message_size+=strlen("POST / HTTP/1.0\r\n");
        message_size+=strlen("Content-Type: application/json");
        message_size+=strlen("\r\n");
        message_size+=strlen("Content-Length: \r\n")+strlen(body);
        message_size+=strlen("\r\n");
        message_size+=strlen(body);


        // allocate space for the message
        message=malloc(message_size);

        sprintf(message, "POST / HTTP/1.0\r\n");
        strcat(message, "Content-Type: application/json");
        strcat(message, "\r\n");
        sprintf(message+strlen(message), "Content-Length: %ld\r\n", strlen(body));
        strcat(message, "\r\n");

        strcat(message, body);

        // create the socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
                strcpy(error_message, "ERROR opening socket to: ");
                strcat(error_message, host);
                Log(error_message, 1);
        }

        // lookup the ip address
        server = gethostbyname(host);
        if (server == NULL) {
                strcpy(error_message, "ERROR, no such host: ");
                strcat(error_message, host);
                Log(error_message, 1);
        }

        // fill in the structure
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

        // connect the socket
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                strcpy(error_message, "Error connecting to: ");
                strcat(error_message, host);
                Log(error_message, 1);
        }

        // send the request
        total = strlen(message);
        sent = 0;
        do {
                bytes = write(sockfd, message+sent, total-sent);
                if (bytes < 0)
                        Log("ERROR writing message to socket", 1);
                if (bytes == 0)
                        break;
                sent+=bytes;
        } while (sent < total);

        // receive the response
        memset(response, 0, sizeof(response));
        total = sizeof(response)-1;
        received = 0;
        do {
                bytes = read(sockfd, response+received, total-received);
                if (bytes < 0)
                        Log("ERROR reading response from socket", 1);
                if (bytes == 0)
                        break;
                received+=bytes;
        } while (received < total);

        if (received == total)
                Log("ERROR storing complete response from socket", 1);

        // close the socket
        close(sockfd);

        free(message);
}

void process()
{
        int count = 0;
        double total = 0;
        char body[256];
        char loadAvgString[256];
        long double a[4], b[4], loadavg;
        FILE *file;

        time_t t;
        struct tm * timeInfo;
        char timeString [20];
        char dateString [20];

        struct timespec ts;
        ts.tv_sec = milSec / 1000;
        ts.tv_nsec = (milSec % 1000) * 1000000;

        for(;;) {
                file = fopen("/proc/stat", "r");
                fscanf(file, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
                fclose(file);
                nanosleep(&ts, NULL);

                file = fopen("/proc/stat", "r");
                fscanf(file, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
                fclose(file);

                loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3])) * 100;

                total += loadavg;
                count++;

                if(count == 9) {
                        sprintf(loadAvgString, "%.2f", (total / 10));

                        strcpy(body, "{\"value\": ");
                        strcat(body, loadAvgString);
                        strcat(body, ", ");
                        strcat(body, "\"date\": \"");

                        time(&t);
                        timeInfo = localtime(&t);
                        strftime (dateString, sizeof(dateString), "%F", timeInfo);
                        strftime (timeString, sizeof(timeString), "%T", timeInfo);
                        strcat(dateString, "T");
                        strcat(dateString, timeString);
                        strcat(body, dateString);

                        strcat(body, "\"}");

                        request(body);

                        count = 0;
                        total = 0;
                }
        }
}

int main()
{
        // Our process ID and Session ID
        pid_t pid, sid;

        // Fork off the parent process
        pid = fork();
        if (pid < 0) {
                exit(EXIT_FAILURE);
        }
        // If we got a good PID, then
        // we can exit the parent process.
        if (pid > 0) {
            exit(EXIT_SUCCESS);
        }

        // Change the file mode mask
        umask(0);


        // Create a new SID for the child process
        sid = setsid();
        if (sid < 0) {
                // Log the failure
                exit(EXIT_FAILURE);
        }

        // Change the current working directory
        if ((chdir("/")) < 0) {
                // Log the failure
                exit(EXIT_FAILURE);
        }

        // Close out the standard file descriptors
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        // Daemon-specific initialization goes here

        // The Big Loop
        // In this case we are just going to call a function and do the loop there.
        process();

        exit(EXIT_SUCCESS);
}
