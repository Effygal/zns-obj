#include <iostream>
#include "rpc/client.h"
#include "data.hpp"

void serializeKVRequest(const KVRequest& request, char* buffer, size_t bufferSize)
{
    // Check if the buffer is large enough to hold the serialized data
    if (bufferSize < sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE) {
        // Handle error (e.g., throw an exception or return an error code)
        return;
    }

    // Serialize request_type
    memcpy(buffer, &request.request_type, sizeof(RequestType));
    buffer += sizeof(RequestType);

    // Serialize key
    memcpy(buffer, &request.key, sizeof(int32_t));
    buffer += sizeof(int32_t);

    // Serialize value
    memcpy(buffer, request.value, BLOCK_SIZE);
}

// Function to convert byte buffer to std::string
std::string bufferToString(const char* buffer, size_t bufferSize)
{
    return std::string(buffer, buffer + bufferSize);
}

// Define the RPC client
void send_request_to_gateway(const KVRequest& request) {
    // Serialize the request
    constexpr size_t bufferSize = sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    serializeKVRequest(request, buffer, bufferSize);
    // Convert the byte buffer to std::string
    std::string serializedData = bufferToString(buffer, bufferSize);

    // Send the serialized request to the gateway
    if (request.request_type == 1) {
    	rpc::client client("127.0.0.1", 5555);	    
    	client.call("HandleRead", serializedData);
    }
    else if (request.request_type == 2) {
        rpc::client client("127.0.0.1", 6666);
    	client.call("HandleWrite", serializedData);
    }
    else if (request.request_type == 3) {
    	rpc::client client("127.0.0.1", 7777);
	    client.call("HandleDel", serializedData);
    }
	
}

// Function to parse command and extract request type, key, and value
bool parseCommand(const std::string& command, KVRequest& request)
{
    // Convert the command string to uppercase for case-insensitive comparison
    std::string upperCommand = command;
    std::transform(upperCommand.begin(), upperCommand.end(), upperCommand.begin(), ::toupper);

    // Find the position of the opening parenthesis
    auto openParenPos = upperCommand.find('(');

    // Find the position of the closing parenthesis
    auto closeParenPos = upperCommand.find(')');

    // Check if parentheses are found
    if (openParenPos != std::string::npos && closeParenPos != std::string::npos) {
        // Extract the command type (GET, PUT, DEL)
        std::string commandType = upperCommand.substr(0, openParenPos);

        // Set the request type based on the command type
        if (commandType == "GET") {
            request.request_type = GET;
            // Check if there's anything after the closing parenthesis
            if (upperCommand.substr(closeParenPos + 1).find_first_not_of(" \t\n\v\f\r") != std::string::npos) {
                std::cerr << "GET command should have only one argument (the key)\n";
                return false;
            }
            // Check if there are any commas between the arguments
            if (std::count(upperCommand.begin(), upperCommand.end(), ',') > 0) {
                std::cerr << "GET command should have only one argument (the key)\n";
                return false;
            }
        } else if (commandType == "PUT") {
            request.request_type = PUT;
            // Check if there's exactly one comma between the arguments
            if (std::count(upperCommand.begin(), upperCommand.end(), ',') != 1) {
                std::cerr << "PUT command should have two arguments (the key and the value)\n";
                return false;
            }
        } else if (commandType == "DEL") {
            request.request_type = DEL;
            // Check if there's anything after the closing parenthesis
            if (upperCommand.substr(closeParenPos + 1).find_first_not_of(" \t\n\v\f\r") != std::string::npos) {
                std::cerr << "DEL command should have only one argument (the key)\n";
                return false;
            }
            // Check if there are any commas between the arguments
            if (std::count(upperCommand.begin(), upperCommand.end(), ',') > 0) {
                std::cerr << "DEL command should have only one argument (the key)\n";
                return false;
            }
        } else {
            std::cerr << "Invalid command type\n";
            return false;
        }

        // Extract the key
        std::string keyStr = upperCommand.substr(openParenPos + 1, closeParenPos - openParenPos - 1);
        try {
            request.key = std::stoi(keyStr);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid key format\n";
            return false;
        } catch (const std::out_of_range& e) {
            std::cerr << "Key out of range\n";
            return false;
        }

        // If PUT or DEL command, check if there's any value specified
        if (request.request_type == PUT) {
            auto commaPos = upperCommand.find(',');
            if (commaPos == std::string::npos) {
                std::cerr << "Command should have two arguments (the key and the value)\n";
                return false;
            }
        }

        return true;
    } else {
        std::cerr << "Invalid command format\n";
        return false;
    }
}



int main() {

    std::string command;
    KVRequest request;

    while (true) {
        // Get user input for command
        std::getline(std::cin, command);

        // Parse command and extract request type, key, and value
        if (!parseCommand(command, request)) {
            continue; // Ask for input again if parsing fails
        }

        // Send the request to the gateway
        send_request_to_gateway(request);
    }

    return 0;
}
