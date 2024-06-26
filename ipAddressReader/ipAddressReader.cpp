#include "ipAddressReader.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

namespace IPAddressReader
{

std::string retrieveHostName()
{
    std::string hostName;
    char host[256];
    if (gethostname(host, sizeof(host)) == 0)
    {
        hostName = std::string(host);
    }
    else
    {
        hostName = "Unknown";
    }

    return hostName;
}    // namespace std::string

std::string retrieveIPAddress()
{
    std::string ipAddress;
    std::string hostName = retrieveHostName();
    if (hostName != "Unknown")
    {
        struct hostent* hostinfo;
        if ((hostinfo = gethostbyname(hostName.c_str())) != NULL)
        {
            ipAddress = std::string(inet_ntoa(*(struct in_addr*)hostinfo->h_addr_list[0]));
        }
        else
        {
            ipAddress = "Unknown";
        }
    }
    else
    {
        ipAddress = "Unknown";
    }

    return ipAddress;
}
}    // namespace IPAddressReader