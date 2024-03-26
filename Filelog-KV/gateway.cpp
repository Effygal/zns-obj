#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "rpc/server.h"
#include "rpc/client.h"
#include <thread> 
#include <vector> 
#include "data.hpp"

std::map<key_t,LBAs> K_LBAs;

void HandleRead() {
    //TODO:
    //read from local K-LBAs map
    rpc::client rc("localhost", 5555);
    rc.call("Read", ).as<void>();
}


void HandleWrite(cmd command) {
    //TODO:
    //first connect to logger to submit the write request
    //submit Append cmd to 3 loggers (currently only 3, fixed, no need to worry about hashing for now)
    rpc::client wc("localhost", 6666);

    LogEnt logent;
    logent.set_key(command.key);
    logent.set_length(sizeof(command.value)); //TODO: check if this is the correct way to get the value's byte
    logent.set_value(command.value);


    off_t lba = wc.call("Append", logent).as<off_t>();
    //then update local K-LBAs map
    K_LBAs[command.key].lbas[0] = lba;
}

void HandleBroadcast(key_t key, off_t lba) {

}

int main() {
    

    std::thread read_thread([]() {
        rpc::server rsrv(5555);
        rsrv.bind("HandleRead", [](int a, int b) {
            return a + b;
        });
        rsrv.run();
    });

    std::thread catchup_thread([]() {
        rpc::server csrv(7777);
        csrv.bind("CatchupMeeple", [](int a, int b) {
            return a + b;
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



