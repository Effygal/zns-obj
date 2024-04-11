#ifndef GATEWAY_H
#define GATEWAY_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "rpc/server.h"
#include "rpc/client.h"
#include <thread> 
#include <pthread.h> // pthread_mutex_t
#include <vector> 
#include "data.hpp"
#include "read_config.hpp"

struct logger {
    std::string ip;
    // below two only for local test, when distributed, can be removed.
    int rport;
    int wport;
};

struct gw {
    std::string ip; 
    // below two only for local test, when distributed, can be removed.
    int bport;
    int cport;
};

class Gateway {
private:
    std::map<key_t, LBAs> K_LBAs;
    pthread_mutex_t _mutex;
public:
    Gateway(){
        pthread_mutex_init(&_mutex, NULL);
        K_LBAs[0] = LBAs{{0,0,0}};
        K_LBAs[1] = LBAs{{0,0,0}};
        K_LBAs[2] = LBAs{{0,0,0}};
    };
    ~Gateway(){
        K_LBAs.clear();
        pthread_mutex_destroy(&_mutex);
    };
    void HandleRead(KVRequest command);
    void HandleWrite(KVRequest  command);
    void HandleBroadcast(key_t key, LBAs lbas);
    void HandleCatchup(key_t key, LBAs lbas);

    std::vector<logger> known_loggers;
    std::vector<gw> known_peers;
    int bport;
    int cport;
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
void TranslateKVReq(KVRequest request, cmd& cmnd);

// Chosse a random logger
int ChooseRandLogger(int min, int max);

// Make a tuple of IP address and ports
std::tuple<std::string, int, int> parseAddress(const std::string& addressStr);

#endif // GATEWAY_H
