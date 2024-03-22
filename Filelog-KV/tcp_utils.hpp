#ifndef TCP_UTILS_HPP
#define TCP_UTILS_HPP

#include <string>

void tcp_server(unsigned short port);
void tcp_client(const std::string& host, unsigned short port, const std::string& message);

#endif // TCP_UTILS_HPP
