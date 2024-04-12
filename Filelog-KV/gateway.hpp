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
    std::vector<gw> failed_peers;
public:
    Gateway(){
        pthread_mutex_init(&_mutex, NULL);
        K_LBAs[0] = LBAs{{0,0,0}};
        K_LBAs[1] = LBAs{{0,0,0}};
        K_LBAs[2] = LBAs{{0,0,0}};//need fix
    };
    ~Gateway(){
        K_LBAs.clear();
        pthread_mutex_destroy(&_mutex);
    };
    std::string HandleRead(KVRequest command);
    std::string HandleWrite(KVRequest  command);
    void HandleBroadcast(key_t key, LBAs lbas);
    void HandleCatchup(key_t key, LBAs lbas);
    void HandleRecovery();
    std::vector<logger> known_loggers;
    std::vector<gw> known_peers;
    int bport;
    int cport;
};

#endif // GATEWAY_H
