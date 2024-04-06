#ifndef READ_CONFIG_HPP
#define READ_CONFIG_HPP

#include <string>
#include <vector>
#include <random>

// Define a struct to hold gateway and logger addresses
struct Config {
    std::vector<std::string> gateways;
    std::vector<std::string> loggers;
};

// Declare the function prototype for reading configuration from a file
Config parseConfig(const std::string& filename);

#endif

