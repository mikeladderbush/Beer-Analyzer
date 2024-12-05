#if defined(__linux__)

#include "linuxSocket.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>

int startSocketLinux();
void error(const char *msg);

// Error for linux sockets
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

// Linux sockets
int startSocketLinux()
{

    // file descriptor variables and port numbers along with return value for read and write calls "n"
    int sockfd, portno, n;

    // structs that contain the internet address as well as the port and AF_INET
    struct sockaddr_in serv_addr;

    // points to a host server on the internet.
    struct hostent *server;

    // convert the second argument to an integer to represent the port.
    // then use the socket function to open the socket and assign it to sockfd.
    portno = atoi("80");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    // take our first argument and assign it as the hostname for our server struct.
    // using this method returns a pointer to a hostent struct that contains data about the passed hostname.
    // the struct holds the ip address in the field char *h_addr
    server = gethostbyname("example.com");
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host");
        exit(0);
    }

    // This code sets the fields in the serv_addr struct which is the struct that contains the server address
    // it does so by zeroing the memory location of the serv_addr struct.
    // then sets the protocol.
    // then by copying the server member "h_addr" to the serv_addr member sin_addr's member s_addr.
    // and finally specifying that the length is equivalent to the server names length.
    // e.g. void bcopy(char *s1, char *s2, int lenght)
    // htons changes the previously provided port member to the correct network byte order.
    char buffer[256];
    memset(buffer, 0, 256);
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    // after establishing the socket and server we want to connect to we use the connect method.
    // which takes the socket file descriptor, address of the host and the length of the address as
    // parameters.
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Error connecting");
    }

    // Now if all has worked correctly we can use the socket and connection to send a message to the server.
    printf("Please enter the message: ");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);
    n = send(sockfd, buffer, strlen(buffer), 0);
    if (n < 0)
    {
        error("Error writing to socket");
    }
    bzero(buffer, 256);
    n = recv(sockfd, buffer, 255, 0);
    if (n < 0)
    {
        error("Error reading from socket");
    }
    printf("%s", buffer);
    return 0;
}

#endif