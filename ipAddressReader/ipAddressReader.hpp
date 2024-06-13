#ifndef IP_ADDRESS_READER_HPP
#define IP_ADDRESS_READER_HPP


#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>


class IpAddressReader {

public:
    IpAddressReader();
    ~IpAddressReader();
    int readIp();





};

#endif
