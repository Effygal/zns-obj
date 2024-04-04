#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
//#include <asio.hpp>
#include <cstring>
#include <thread>
#include <queue>
#include <pthread.h> // pthread_mutex_t
#include "data.hpp"
#include "rpc/server.h"
#include "rpc/client.h"



class Logger {
public:
    Logger();
    ~Logger();

    off_t Append(const LogEnt& logent);
    void Read(key_t key, void* buff);

private:
    ssize_t _len;
    int _fd;
    off_t _cur_lba;
    pthread_mutex_t _mutex;
    int16_t _logger_id;
    std::vector<std::pair<std::string, int>> _gws; // gateway servers, <ip, port>
    int _wport;
    int _rport;

    void BroadcastLBA(key_t key, off_t lba);
};

#endif // LOGGER_HPP
