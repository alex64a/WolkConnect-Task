#ifndef IPADDRESSREADER_HPP
#define IPADDRESSREADER_HPP

#include <string>

class IPAddressReader {
public:
    IPAddressReader();
    std::string getHostName();
    std::string getIPAddress();

private:
    std::string hostName;
    std::string ipAddress;

    void retrieveHostName();
    void retrieveIPAddress();
};

#endif // IPADDRESSREADER_HPP

