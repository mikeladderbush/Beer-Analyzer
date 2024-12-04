// Function declarations and lack of includes due to cross platform
int startSocketWindows(int argc, char* argv[]);
int startSocketLinux(int argc, char* argv[]);
void error(const char *msg);

#if defined(_WIN32) || defined(_WIN64)

// Using the WIN32_LEAN_AND_MEAN macro in order to avoid including winsock.h in the windows header	  //, which would cause linking errors.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>

// Linking with the Ws2_32.lib library, doesn't work with GCC or MinGW.
#pragma comment(lib, "Ws2_32.lib")

#elif defined(__linux__)

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>

#else
#error "Unsupported platform"
#endif

int main(int argc, char *argv[])
{

#if defined(_WIN32) || defined(_WIN64)

    int result = startSocketWindows(argc, argv);
    if (result == 1){
        printf("Windows socket connection failed");
        return 1;
    }

#elif defined(__linux__)

    int result = startSocketLinux(argc, argv);
    if (result == 1) {
        printf("Linux socket connection failed");
        return 1;
    }

#else
#error "Unsupported platform"
#endif

    return 0;
}

#if defined(_WIN32) || defined(_WIN64)

int startSocketWindows(int argc, char *argv[])
{

    // I need to use sockets in order to send and receive data over a network.
    // To use sockets in C++ I need to use a library like Winsock from microsoft.
    // Winsock will allow me to create sockets.
    // Initializing Winsock by creating a WSADATA struct which will define the implementation of my sockets.
    WORD wVersionRequested;
    WSADATA wsaData;
    int iResult;

    // Here I use MAKEWORD to concatenate two unsigned shorts. This respresents version 2.2 of winsock and needs to be provided to the struct WSADATA.
    wVersionRequested = MAKEWORD(2, 2);

    iResult = WSAStartup(wVersionRequested, &wsaData);
    if (iResult != 0)
    {
        // Cannot find a usable DLL.
        printf("WSAStartup failed with an error: %d\n", iResult);
        return 1;
    }

    // Determine the version of Winsock by checking the high and low bytes of the wVersion.
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        // Cannot find the correct DLL.
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return 1;
    }
    else
    {
        printf("Winsock 2.2 dll was found\n");
    }

    // Defining the address info struct which will contain multiple values withins "hints".
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    // Fills the memloc of hints with zeros, then defines the family, socket type and protocol.
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

#define DEFAULT_PORT "80"

    // Resolve the server address and port by providing the node via the command line along with other parameters.
    // Will update and replace node argument with constant.
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Now we can create a socket.
    SOCKET ConnectSocket = INVALID_SOCKET;

    // Attempt to connect to the first address returned by
    // the call to getaddrinfo
    ptr = result;

    // Create a socket for connecting to the server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    // Error checking that specifically returns the last error in the socket.
    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Error at socket(): ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Connect to the server by passing a socket and the variables of a sockaddr struct
    // and if fails then close socket and set socket to Invalid.
    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    // Typically the next address will be attempted but documentation says
    // This example just keeps it simple.

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to the server\n");
        WSACleanup();
        return 1;
    }

// Defining new buffer variables for when we receive data from the server.
#define DEFAULT_BUFLEN 512

    int recvbuflen = DEFAULT_BUFLEN;

    const char *sendbuf = "this is a test\n";
    char recvbuf[DEFAULT_BUFLEN];

    // Testing a send function to the socket.
    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed; %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Send: %ld\n", iResult);

    // Now I will shutdown the socket and leave it open only for receiving.
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Now the socket will only receive data until the connection is closed.
    do
    {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            printf("Bytes received: %d\n", iResult);
        }
        else if (iResult == 0)
        {
            printf("Connection Closed\n");
        }
        else
        {
            printf("recv failed: $d\n", WSAGetLastError());
        }
    } while (iResult > 0);

    WSACleanup();
    return 0;
}
#endif

#if defined(__linux__)
// Error for linux sockets
void error(const char *msg)
{
    perror(msg);
    exit(0);
}
// Linux sockets
int startSocketLinux(int argc, char *argv[])
{

    // file descriptor variables and port numbers along with return value for read and write calls "n"
    int sockfd, portno, n;

    // structs that contain the internet address as well as the port and AF_INET
    struct sockaddr_in serv_addr;

    // points to a host server on the internet.
    struct hostent *server;

    // Create a buffer to store the requests and determine if the correct arguments are provided.
    char buffer[256];
    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port", argv[0]);
        exit(0);
    }

    // convert the second argument to an integer to represent the port.
    // then use the socket function to open the socket and assign it to sockfd.
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    // take our first argument and assign it as the hostname for our server struct.
    // using this method returns a pointer to a hostent struct that contains data about the passed hostname.
    // the struct holds the ip address in the field char *h_addr
    server = gethostbyname(argv[1]);
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
