#ifndef UDP_UTILS_HPP
#define UDP_UTILS_HPP

#include <string>

void udp_server(unsigned short port);
void udp_client(const std::string& host, unsigned short port, const std::string& message);

#endif // UDP_UTILS_HPP
