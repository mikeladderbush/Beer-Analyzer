#if defined(_WIN32) || defined(_WIN64)

#include "windowsSocket.h"

#elif defined(__linux__)

#include "linuxSocket.h"

#else
#error "Unsupported platform"
#endif

#include <stdio.h>

int main()
{

#if defined(_WIN32) || defined(_WIN64)

    int result = startSocketWindows();
    if (result == 1){
        printf("Windows socket connection failed");
        return 1;
    }

#elif defined(__linux__)

    int result = startSocketLinux();
    if (result == 1) {
        printf("Linux socket connection failed");
        return 1;
    }

#else
#error "Unsupported platform"
#endif

    return 0;
}


