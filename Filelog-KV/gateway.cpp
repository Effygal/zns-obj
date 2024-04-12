/*
    ------------------------------------
    Gateways
    ------------------------------------
    Usage: ./gateway <gateway_id(0-2)>
    ------------------------------------
    Loggers are supposed to not fail because they are persistent storage.
*/

#include "gateway.hpp"
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;

std::string Gateway::HandleWrite(KVRequest command) {
    LBAs lbas;
    cmd cmnd;
    cmnd.op = 2;
    cmnd.key = command.key;
    std::memcpy(cmnd.value, command.value, BLOCK_SIZE);
    
    for (auto logger : known_loggers) {
        std::cout << "Sending to logger: " << logger.ip << logger.wport << std::endl;
        
        try {
            rpc::client ac(logger.ip, logger.wport);
            auto reply = ac.call("Append", cmnd);
            AppendReply append_reply = reply.as<AppendReply>();
            
            lbas.lbas[append_reply._logger_id] = append_reply.lba;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            std::cout << "Failed to send to logger: " << logger.ip << logger.wport << std::endl;
            continue;
        }
    }

    K_LBAs[command.key] = lbas;
    HandleBroadcast(command.key, lbas);
    
    std::cout << "Write successful" << std::endl;
    std::cout << "Key: " << command.key << std::endl;
    
    for (int i = 0; i < 3; i++) {
        std::cout << "LBA " << i << ": " << lbas.lbas[i] << std::endl;
    }
    
    return "Success";
}


std::string Gateway::HandleRead(KVRequest command) {
    std::string msg;
    bool valid_reply_found = false;

    int random = rand() % known_loggers.size();
    
    for (random = random % known_loggers.size(); random < known_loggers.size(); random++) {
        logger logger = known_loggers[random];
        ReadReply reply;
        try {
            rpc::client rc(logger.ip, logger.rport);
            cmd cmnd;
            cmnd.op = 1;
            cmnd.key = command.key;
            reply = rc.call("Read", cmnd, K_LBAs[command.key].lbas[random]).as<ReadReply>();
            msg = reply.value;  
            valid_reply_found = true;  
            std::cout << "Read value: " << msg << std::endl;
            break;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            std::cout << "Failed to read from logger: " << logger.ip << logger.rport << std::endl;
            std::cout << "Trying next logger" << std::endl;
            continue;
        }
    }

    if (!valid_reply_found) {
        std::cerr << "Failed to read from any logger" << std::endl;
        // Handle the case where no valid reply was found
    }

    return msg;
}


// void Gateway::HandleBroadcast(key_t key, LBAs lbas) {
//     for (auto peer : known_peers) {
//         try {
//             rpc::client bc(peer.ip, peer.bport);
//             bc.call("HandleCatchup", key, lbas); 
//         } catch (const std::exception& e) {
//             std::cerr << "Error: " << e.what() << std::endl;
//             std::cout << "Failed to broadcast to peer: " << peer.ip <<" "<< peer.bport << std::endl;
//             failed_peers.push_back(peer);
//             continue;
//         }
//     }
// }
void Gateway::HandleBroadcast(key_t key, LBAs lbas) {
    for (auto peer : known_peers) {
        try {
            rpc::client bc(peer.ip, peer.bport);
            bc.call("HandleCatchup", key, lbas); 
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            std::cout << "Failed to broadcast to peer: " << peer.ip <<" "<< peer.bport << std::endl;
            // Add the failed peer to the failed_peers vector
            {
                std::unique_lock<std::mutex> lock(mtx);
                failed_peers.push_back(peer);
            } 
            cv.notify_one();
            continue;
        }
    }
}

void Gateway::HandleCatchup(key_t key, LBAs lbas) {
    // Update the K_LBAs map
    // compare the lbas with the existing ones
    pthread_mutex_lock(&_mutex);
    for (int i = 0; i < 3; i++) {
        if (lbas.lbas[i] > K_LBAs[key].lbas[i]) {
            K_LBAs[key].lbas[i] = lbas.lbas[i];
        }
    pthread_mutex_unlock(&_mutex);
    }
}

//Lazy recovery
// void Gateway::HandleRecovery() {
//     while (failed_peers.size() > 0){
//         pthread_mutex_lock(&_mutex);
//         for (auto peer : failed_peers) {
//             std::cout << "Recovering peer: " << peer.ip << peer.bport << std::endl;
//                 try {
//                     rpc::client rc(peer.ip, peer.bport);
//                     for (auto entry : K_LBAs) {
//                         rc.call("HandleCatchup", entry.first, entry.second);
//                     }
//                     failed_peers.erase(std::remove(failed_peers.begin(), failed_peers.end(), peer), failed_peers.end());
//                 } catch (const std::exception& e) {
//                     continue;
//                 }
//         }
//         pthread_mutex_unlock(&_mutex);
//     }
// }
void Gateway::HandleRecovery() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        // Wait until there are failed peers to recover
        cv.wait(lock, [this]() { return !failed_peers.empty(); });

        for (auto it = failed_peers.begin(); it != failed_peers.end();) {
            auto peer = *it;

            std::cout << "Recovering peer: " << peer.ip << peer.bport << std::endl;
            try {
                rpc::client rc(peer.ip, peer.bport);
                for (auto entry : K_LBAs) {
                    rc.call("HandleCatchup", entry.first, entry.second);
                }
                it = failed_peers.erase(it); // Erase the recovered peer
            } catch (const std::exception& e) {
                ++it; // Move to the next failed peer
                continue;
            }

        }

    }
}


int main(int argc, char** argv) {
    int gatewayID = atoi(argv[1]);//0-2
    Gateway gateway;
    gateway.bport = 5500 + gatewayID;
    gateway.cport = 6600 + gatewayID;
    // Load the configuration file later
    int peer1ID = (gatewayID + 1) % 3;
    int peer2ID = (gatewayID + 2) % 3;
    gateway.known_peers.push_back(gw{"127.0.0.1", 5500+peer1ID, 6600+peer1ID});
    gateway.known_peers.push_back(gw{"127.0.0.1", 5500+peer2ID, 6600+peer2ID});
    gateway.known_loggers.push_back(logger{"127.0.0.1", 5000, 6000});
    gateway.known_loggers.push_back(logger{"127.0.0.1", 5001, 6001});
    gateway.known_loggers.push_back(logger{"127.0.0.1", 5002, 6002});
    std::cout << "Gateway " << gatewayID << " started" << std::endl;

    std::thread server_thread([&gateway]() {
        rpc::server srv(gateway.cport); // Use the same port for all services
        std::cout << "listening on port " << gateway.cport << " for serveice..." << std::endl;
        srv.bind("HandleRead", [&gateway](KVRequest command) -> std::string{
            std::string reply = gateway.HandleRead(command);
            return reply;
        });

        srv.bind("HandleWrite", [&gateway](KVRequest command) -> std::string{
            std::string reply = gateway.HandleWrite(command);
            return reply;
        });

        srv.run();
    });

    std::thread catchup_thread([&gateway]() {
        rpc::server bsrv(gateway.bport);
        std::cout << "listening on port " << gateway.bport << " for broadcast catch up..." << std::endl;
        bsrv.bind("HandleCatchup", [&gateway](key_t key, LBAs lbas) {
            gateway.HandleCatchup(key, lbas);
        });
        
        bsrv.run();
    });

    std::thread recovery_thread([&gateway]() {
        gateway.HandleRecovery();
    });

    server_thread.join();
    catchup_thread.join();
    recovery_thread.join();
    return 0;
}