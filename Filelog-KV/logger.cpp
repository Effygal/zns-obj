#include <iostream>
#include <asio.hpp>
#include <string>
#include <thread>
#include "data.hpp"
#include "rpc/server.h"
#include "rpc/client.h"

class Logger {

	public:
		Logger() : _cur_lba(0) {
            pthread_mutex_init(&_mutex, nullptr);
        }

        ~Logger() {
            pthread_mutex_destroy(&_mutex);
        }
        off_t Append(const LogEnt& logent) {
            pthread_mutex_lock(&_mutex);
            pwrite(_fd, &logent, sizeof(LogEnt), _cur_lba);
            off_t offset = lseek(_fd, sizeof(LogEnt), SEEK_END);
            _cur_lba = offset;
            pthread_mutex_unlock(&_mutex);
            BroadcastLBA(logent.key(), offset);
            return offset;
        }
        void Read(key_t key, void* buff) {
            off_t lba = K_LBAs[key].lbas[_logger_id] - sizeof(LogEnt);
            pread(_fd, buff, sizeof(LogEnt), lba);
        }

        void BroadcastLBA(key_t key, off_t lba) {
            std::queue<rpc::future> futures;
            for gw in _gws;
                rpc::Client client= new rpc::client(gw.first, gw.second);
                rpc::Future future= rpc::client.asyncCall("HandleBroadcast", key, lba);
                futures.push(future);
            for future in futures;
                future.join();
        }   

		private:
			ssize_t _len;
			int _fd;
			off_t _cur_lba;
			pthread_mutex_t _mutex;
            int16_t _logger_id;
            std::vector<std::pair<std::string, int>> _gws; // gateway servers, <ip, port>
            int _wport;
            int _rport;

};

int main() {
    Logger logger;
    //TODO: dispatch below on a separate threads
    rpc::server asrv(_wport);
    asrv.bind("Append", [&](const LogEnt& logent) {
        return logger.Append(logent);
    });
    asrv.run();

    rpc::server rsrv(_rport);
    rsrv.bind("Read", [&](key_t key, void* buff) {
        return logger.Read(key, buff);
    });
    return 0;
}