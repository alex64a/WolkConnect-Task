#include "ipAddressReader.hpp"

IpAddressReader::IpAddressReader()
{
}

IpAddressReader::~IpAddressReader()
{
}

int IpAddressReader::readIp()
{

    char host[256];
    char ip[256];

    if (gethostname(host, sizeof(host)) == 0) {
        struct hostent *hostinfo;
        if ((hostinfo = gethostbyname(host)) != NULL) {
            strcpy(ip, inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[0]));
            std::cout << "Hostname: " << host << std::endl;
            std::cout << "IP Address: " << ip << std::endl;
        } else {
            std::cerr << "Error: Unable to get IP address." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Error: Unable to get hostname." << std::endl;
        return 1;
    }

    return 0;
}
