#ifndef IPADDRESSREADER_HPP
#define IPADDRESSREADER_HPP

#include <string>

class IPAddressReader
{
public:
    IPAddressReader();
    std::string retrieveHostName();
    std::string retrieveIPAddress();

private:
    std::string hostName;
    std::string ipAddress;
};

#endif    // IPADDRESSREADER_HPP
