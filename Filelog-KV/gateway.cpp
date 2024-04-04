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

class Gateway{
    public:
        Gateway();
        ~Gateway();
        void HandleRead(cmd command) {

            LBAs lbas = K_LBAs[command.key];
            off_t lba0 = lbas.lbas[0];
            rpc::client rc();
            off_t lba1 = lbas.lbas[1];
            off_t lba2 = lbas.lbas[2];

            //TODO: for each logger, do these stuff...
            // rpc::client rc(ip, port);
            // rc.call("HandleRead", command);

        }
        off_t HandleWrite(cmd command) {
        //TODO:
            //first connect to logger to submit the write request
            //submit Append cmd to all known loggers (currently only 3, fixed, no need to worry about hashing for now)
            // for each logger, do these stuff...
            // rpc::client wc(ip, wport);
            // LogEnt logent;
            // logent.set_key(command.key);
            // logent.set_length(sizeof(command.value)); //TODO: check if this is the correct way to get the value's byte
            // logent.set_value(command.value);

            // off_t lba = wc.call("Append", logent).as<off_t>();
            // //then update local K-LBAs map
            // K_LBAs[command.key].lbas[0] = lba;
        }
        void HandleBroadcast(key_t key, off_t lba) {
            //TODO: Broadcast to other gateways
        }
        void HandleCatchup(key_t key, off_t lba) {
            //TODO: Catchup from other gateways
            //this is one particular thread listening to catchup requests...
        }
        std::vector<logger> known_loggers;
        std::vector<gw> known_peers;
    private:
        std::map<key_t,LBAs> K_LBAs;        
};

int main() {
    

    std::thread read_thread([]() {
        rpc::server rsrv(5555);
        rsrv.bind("HandleRead", []() {
          
        });
        rsrv.run();
    });

    std::thread catchup_thread([]() {
        rpc::server csrv(7777);
        csrv.bind("CatchupMeeple", []() {
        });
        csrv.run();
    });

    constexpr int num_write_threads = 4; // Adjust this as needed
    std::vector<std::thread> write_thread_pool;
    for (int i = 0; i < num_write_threads; ++i) {
        write_thread_pool.emplace_back([]() {
            rpc::server wsrv(6666);
            wsrv.bind("HandleWrite", [](int a, int b) {
                return a + b;
            });
            wsrv.run();
        });
    }

    read_thread.join();
    for (auto& thread : write_thread_pool) {
        thread.join();
    }

    return 0;
}



