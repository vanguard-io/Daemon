void request(const char *host, int port, const char *method, const char *path, const char *body)
{
        int i;

        char error_message[1000] = "";

        struct hostent *server;
        struct sockaddr_in serv_addr;
        int sockfd, bytes, sent, received, total, message_size;
        char *message, response[4096];

        // How big is the message?
        message_size=0;
        message_size+=strlen(method);
        message_size+=strlen(path);
        message_size+=strlen("  HTTPS/1.0\r\n");
        message_size+=strlen("Content-Type: application/json");
        message_size+=strlen("\r\n");
        message_size+=strlen("Content-Length: \r\n")+strlen(body);
        message_size+=strlen("\r\n");
        message_size+=strlen(body);


        // allocate space for the message
        message=malloc(message_size);

        strcpy(message, method);
        strcat(message, " ");
        strcat(message, path);
        strcat(message, " HTTP/1.0\r\n");
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
                Log(error_message, TRUE);
        }

        // lookup the ip address
        server = gethostbyname(host);
        if (server == NULL) {
                strcpy(error_message, "ERROR, no such host: ");
                strcat(error_message, host);
                Log(error_message, TRUE);
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
                Log(error_message, TRUE);
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
