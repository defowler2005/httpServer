#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "Ws2_32.lib")

static std::string getLocalIPv4()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed." << std::endl;
        return "";
    }

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
    {
        std::cerr << "Error getting hostname." << std::endl;
        WSACleanup();
        return "";
    }

    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *result = nullptr;
    if (getaddrinfo(hostname, nullptr, &hints, &result) != 0)
    {
        std::cerr << "Error getting IP address." << std::endl;
        WSACleanup();
        return "";
    }

    std::string ipv4;
    for (struct addrinfo *ptr = result; ptr != nullptr; ptr = ptr->ai_next)
    {
        struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *)ptr->ai_addr;
        char ipstr[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ipstr, INET_ADDRSTRLEN);
        ipv4 = std::string(ipstr);
        break;
    };
    freeaddrinfo(result);
    WSACleanup();
    return ipv4;
};