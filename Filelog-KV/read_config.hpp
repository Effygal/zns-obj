#ifndef READ_CONFIG_HPP
#define READ_CONFIG_HPP

#include <string>
#include <vector>
#include <random>
#include <sstream>

// Define a struct to hold gateway and logger addresses
struct Config {
    std::vector<std::string> gateways;
    std::vector<std::string> loggers;
};

// Declare the function prototype for reading configuration from a file
Config parseConfig(const std::string& filename);

// Make a pair of IP address and port
std::pair<std::string, int> splitAddress(const std::string& address);

// Make a tuple of IP address and ports
std::tuple<std::string, int, int> parseAddress(const std::string& addressStr);

#endif

