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
    }

    return msg;
}

void Gateway::HandleBroadcast(key_t key, LBAs lbas) {
    for (auto peer : known_peers) {
        try {
            rpc::client bc(peer.ip, peer.bport);
            bc.call("HandleCatchup", key, lbas); 
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            std::cout << "Failed to broadcast to peer: " << peer.ip <<" "<< peer.bport << std::endl;
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
void Gateway::HandleRecovery() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return !failed_peers.empty(); });

        for (auto it = failed_peers.begin(); it != failed_peers.end();) {
            auto peer = *it;
            try {
                rpc::client rc(peer.ip, peer.bport);
                for (auto entry : K_LBAs) {
                    rc.call("HandleCatchup", entry.first, entry.second);
                }
                it = failed_peers.erase(it); 
            } catch (const std::exception& e) {
                ++it; 
                continue;
            }

        }

    }
}


int main(int argc, char** argv) {

    if (argc < 2 ) {
        std::cerr << "Usage: ./gateway <gateway_id(0-2)>" << std::endl;
        return 1;
    }

    // Read config.json file
    Config conf = parseConfig("config.json");

    int gatewayID = atoi(argv[1]);//0-2
    auto result = parseAddress(conf.gateways[gatewayID]);
    Gateway gateway;
    gateway.ip = std::get<0>(result);
    gateway.bport = std::get<1>(result);
    gateway.cport = std::get<2>(result);;

    for (size_t i = 0; i < conf.gateways.size(); ++i) {
        // skip itself
        if (i == gatewayID) continue;
        auto gateway_info = parseAddress(conf.gateways[i]);
        std::string gateway_ip = std::get<0>(gateway_info);
        int gateway_bport = std::get<1>(gateway_info);
        int gateway_cport = std::get<2>(gateway_info);
        gateway.known_peers.push_back(gw{gateway_ip, gateway_bport, gateway_cport});
    }


    std::string logger_ip;
    int logger_rport, logger_wport;
    for (size_t i = 0; i < conf.loggers.size(); ++i) {
        auto logger_info = parseAddress(conf.loggers[i]);
        logger_ip = std::get<0>(logger_info);
        logger_rport = std::get<1>(logger_info);
        logger_wport = std::get<2>(logger_info);
        gateway.known_loggers.push_back(logger{logger_ip, logger_rport, logger_wport});
    }

    std::cout << "Gateway " << gatewayID << " started" << std::endl;

    std::thread server_thread([&gateway]() {
        rpc::server srv(gateway.cport); // Use the same port for all services
        std::cout << "listening on port " << gateway.cport << " for service..." << std::endl;
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