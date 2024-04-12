#ifndef CLIENT_H
#define CLIENT_H
#include <cstring>
#include <iostream>
#include "rpc/client.h"
#include "data.hpp"

// Function to parse command and extract request type, key, and value
bool parseCommand(const std::string& command, KVRequest& request);

#endif // CLIENT_H
