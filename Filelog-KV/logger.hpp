#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <cstring>
#include <thread>
#include <queue>
#include <pthread.h> // pthread_mutex_t
#include "data.hpp"
#include "rpc/server.h"
#include "rpc/client.h"
#include <fcntl.h> // Include the <fcntl.h> header file to define the O_CREAT constant

class Logger {
public:
    
    Logger(){
        _fd = open("log.txt", O_CREAT | O_RDWR, 0666); // Use open() instead of fopen()
        _cur_lba = 0;
        pthread_mutex_init(&_mutex, NULL);
    };
    ~Logger(){
        close(_fd);
        pthread_mutex_destroy(&_mutex);
    };
    off_t Append(const LogEnt& logent);
    void Read(key_t key, void* buff, off_t lba);
    AppendReply AppendThread(cmd& request);
    ReadReply ReadThread(cmd& request, off_t lba);
    int8_t _logger_id;
    int wport;
    int rport;
private:
    ssize_t _len;
    int _fd;
    off_t _cur_lba;
    pthread_mutex_t _mutex;
    std::vector<std::pair<std::string, int>> _gws; // gateway servers, <ip, port>
    
};
#endif // LOGGER_HPP
