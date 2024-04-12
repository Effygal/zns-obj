#include "read_config.hpp"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <fstream>
#include <iostream>

Config parseConfig(const std::string& filename) {
    Config config;

    // Open the configuration file
    std::ifstream configFile(filename);
    if (!configFile.is_open()) {
        std::cerr << "Failed to open configuration file: " << filename << std::endl;
        return config;
    }

    // Read the configuration file into a string
    std::string jsonString((std::istreambuf_iterator<char>(configFile)), std::istreambuf_iterator<char>());

    // Parse the JSON string
    rapidjson::Document document;
    document.Parse(jsonString.c_str());

    // Check if parsing succeeded
    if (document.HasParseError()) {
        std::cerr << "Failed to parse configuration file: " << filename << std::endl;
        return config;
    }

    // Get gateways array
    const rapidjson::Value& gateways = document["gateways"];
    if (gateways.IsArray()) {
        for (rapidjson::SizeType i = 0; i < gateways.Size(); i++) {
            config.gateways.push_back(gateways[i].GetString());
        }
    }

    // Get loggers array
    const rapidjson::Value& loggers = document["loggers"];
    if (loggers.IsArray()) {
        for (rapidjson::SizeType i = 0; i < loggers.Size(); i++) {
            config.loggers.push_back(loggers[i].GetString());
        }
    }

    return config;
}

// Make a pair of IP address and port
std::pair<std::string, int> splitAddress(const std::string& address) {
    std::istringstream iss(address);
    std::string ip, portStr;

    // Get IP address
    std::getline(iss, ip, ':');

    // Get port number
    std::getline(iss, portStr);

    // Convert port string to integer
    int port = std::stoi(portStr);
    std::cout << ip << std::endl;
    std::cout << port << std::endl;

    return std::make_pair(ip, port);
}

// Make a tuple of IP address and ports
std::tuple<std::string, int, int> parseAddress(const std::string& addressStr) {
    // Find the position of the colon
    size_t colonPos = addressStr.find(':');
    if (colonPos == std::string::npos) {
        throw std::invalid_argument("Invalid address format: missing colon");
    }

    // Extract the IP address and port numbers
    std::string ip = addressStr.substr(0, colonPos);
    size_t commaPos = addressStr.find(',');
    if (commaPos == std::string::npos) {
        throw std::invalid_argument("Invalid address format: missing comma");
    }
    int port1 = std::stoi(addressStr.substr(colonPos + 1, commaPos - colonPos - 1));
    int port2 = std::stoi(addressStr.substr(commaPos + 2)); // Skip comma and space

    return std::make_tuple(ip, port1, port2);
}
