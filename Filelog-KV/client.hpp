#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include "rpc/client.h"
#include "data.hpp"

// Serializing the request in order to send it over the network
void serializeKVRequest(const KVRequest& request, char* buffer, size_t bufferSize);

// Function to convert byte buffer to std::string
std::string bufferToString(const char* buffer, size_t bufferSize);

// Define the RPC client
void send_request_to_gateway(const KVRequest& request, std::string ip, int port);

// Function to parse command and extract request type, key, and value
bool parseCommand(const std::string& command, KVRequest& request);

// Chosse a random gateway
int ChooseRandGateway(int min, int max);

// Choose a random number based on uniform distribution
std::pair<std::string, int> splitAddress(const std::string& address);

#endif // CLIENT_H
