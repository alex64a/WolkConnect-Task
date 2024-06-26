#ifndef IPADDRESSREADER_HPP
#define IPADDRESSREADER_HPP

#include <string>

/**
 * @brief Gets the ip address and hostname of local machine
 * @returns std::string that consists of ip address
 */
namespace IPAddressReader
{

std::string retrieveHostName();
std::string retrieveIPAddress();

}    // namespace IPAddressReader

#endif    // IPADDRESSREADER_HPP
