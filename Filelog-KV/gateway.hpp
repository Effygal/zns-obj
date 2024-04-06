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
#include "read_config.hpp"

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


// Function to serialize cmd struct into a byte buffer
void SerializeCMDRequest(const cmd& request, char* buffer, size_t bufferSize);

// Function to convert std::string to byte buffer
void stringToBuffer(const std::string& serializedData, char* buffer, size_t bufferSize);
// Function to convert byte buffer to std::string
std::string bufferToString(const char* buffer, size_t bufferSize);

// Function to deserialize byte buffer into KVRequest struct
void DeserializeKVRequest(const char* buffer, size_t bufferSize, KVRequest& request);

// Process different types of requests
// void ProcessGetRequest(const std::string& serializedData);
//void ProcessPutRequest(const std::string& serializedData);
// void ProcessDelRequest(const std::string& serializedData);
void ProcessRequest(const std::string& serializedData, Config conf);

// Translate KV request to cmd
void TranslateKVReq(KVRequest request);

// Chosse a random logger
int ChooseRandLogger(int min, int max);

// Make a tuple of IP address and ports
std::tuple<std::string, int, int> parseAddress(const std::string& addressStr);

#endif // GATEWAY_H
