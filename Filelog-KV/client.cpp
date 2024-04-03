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

// Define your RPC client
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
    //else if (request.request_type == 3)
    	//rpc::client client("127.0.0.1", 7777);
//	client.call("HandleDel", serializedData);
	
}


int main() {

    while (true) {
        // Get user input for request type, key, and value (if applicable)
        std::string command;
        std::cout << "Enter command (GET, PUT, DELETE): ";
        std::cin >> command;

        KVRequest request;
        if (command == "GET") {
            request.request_type = GET;
        } else if (command == "PUT") {
            request.request_type = PUT;
        } else if (command == "DELETE") {
            request.request_type = DELETE;
        } else {
            std::cerr << "Invalid command\n";
            continue; // Ask for input again
        }

        if (request.request_type == PUT) {
            std::cout << "Enter key: ";
            std::cin >> request.key;
            std::cout << "Enter value: ";
            std::cin >> request.value;
        } else if (request.request_type == GET || request.request_type == DELETE) {
            std::cout << "Enter key: ";
            std::cin >> request.key;
        }

	send_request_to_gateway(request);
    }
    return 0;
}
