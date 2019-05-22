#include <stdio.h> // printf, sprintf
#include <stdlib.h> // exit, atoi, malloc, free
#include <time.h> // nanosleep
#include <unistd.h> // read, write, close
#include <string.h> // memcpy, memset, strcpy
#include <sys/socket.h> // socket, connect
#include <netinet/in.h> // struct sockaddr_in, struct sockaddr
#include <netdb.h> // struct hostent, gethostbyname
#include <sys/stat.h> // umask
#include "Time.h" // nowString
#include "Log.h" // log
#include "Request.h" // request
