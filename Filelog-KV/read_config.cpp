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
