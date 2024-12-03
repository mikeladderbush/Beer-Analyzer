// Using the WIN32_LEAN_AND_MEAN macro in order to avoid including winsock.h in the windows header, which would cause linking errors.
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

int main(int argc, char* argv[]) {

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
    if (iResult != 0){
        // Cannot find a usable DLL.
        printf("WSAStartup failed with an error: %d\n", iResult);
        return 1;
    }

    // Determine the version of Winsock by checking the high and low bytes of the wVersion.
    if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2){
        // Cannot find the correct DLL.
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return 1;
    } else {
        printf("Winsock 2.2 dll was found");
    }

    // Defining the address info struct which will contain multiple values withins "hints".
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    // Fills the memloc of hints with zeros, then defines the family, socket type and protocol.
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    #define DEFAULT_PORT "27015"

    // Resolve the server address and port by providing the node via the command line along with other parameters.
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
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

    WSACleanup();

}