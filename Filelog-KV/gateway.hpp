#ifndef GATEWAY_H
#define GATEWAY_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "rpc/server.h"
#include "rpc/client.h"
#include <thread> 
#include <vector> 
#include "data.hpp"

struct logger {
    std::string ip;
    int wport;
    int rport;
};

struct gw {
    std::string ip;
    int bport;
    int cport;
};

class Gateway {
private:
    std::map<key_t, LBAs> K_LBAs;
public:
    Gateway();
    ~Gateway();
    void HandleRead(cmd command);
    off_t HandleWrite(cmd command);
    void HandleBroadcast(key_t key, off_t lba);
    void HandleCatchup(key_t key, off_t lba);

    std::vector<logger> known_loggers;
    std::vector<gw> known_peers;

};

// Function to convert std::string to byte buffer
void stringToBuffer(const std::string& serializedData, char* buffer, size_t bufferSize);

// Function to deserialize byte buffer into KVRequest struct
void deserializeKVRequest(const char* buffer, size_t bufferSize, KVRequest& request);

// Process different types of requests
void ProcessGetRequest(const std::string& serializedData);
void ProcessPutRequest(const std::string& serializedData);
void ProcessDelRequest(const std::string& serializedData);

#endif // GATEWAY_H
