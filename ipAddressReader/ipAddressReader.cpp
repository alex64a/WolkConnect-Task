#include "ipAddressReader.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

IPAddressReader::IPAddressReader() {
    retrieveHostName();
    retrieveIPAddress();
}

void IPAddressReader::retrieveHostName() {
    char host[256];
    if (gethostname(host, sizeof(host)) == 0) {
        hostName = std::string(host);
    } else {
        hostName = "Unknown";
    }
}

void IPAddressReader::retrieveIPAddress() {
    if (hostName != "Unknown") {
        struct hostent *hostinfo;
        if ((hostinfo = gethostbyname(hostName.c_str())) != NULL) {
            ipAddress = std::string(inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[0]));
        } else {
            ipAddress = "Unknown";
        }
    } else {
        ipAddress = "Unknown";
    }
}

std::string IPAddressReader::getHostName() {
    return hostName;
}

std::string IPAddressReader::getIPAddress() {
    return ipAddress;
}

