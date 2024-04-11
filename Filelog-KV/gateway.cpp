#include "gateway.hpp"
/*
    ------------------------------------
    Added stuff
*/
std::string Gateway::HandleWrite(KVRequest command) {
    // for each logger in known_loggers
    std::cout << "Handling write" << std::endl;
    std::cout << "Key: " << command.key << std::endl;
    std::cout << "Value: " << command.value << std::endl;
    LBAs lbas;
    cmd cmnd;
    cmnd.op = 2;
    cmnd.key = command.key;
    std::memcpy(cmnd.value, command.value, BLOCK_SIZE);
    for (auto logger : known_loggers) {
        std::cout << "Sending to logger: " << logger.ip << logger.wport << std::endl;
        rpc::client ac(logger.ip, logger.wport);
        std::cout << "connected" << std::endl;
        auto reply = ac.call("Append", cmnd);
        AppendReply append_reply = reply.as<AppendReply>();
        std::cout << "Got reply: " << append_reply._logger_id << " " << append_reply.lba << std::endl;
        // Update the K_LBAs map
        lbas.lbas[append_reply._logger_id] = append_reply.lba;
    }
    K_LBAs[command.key] = lbas;
    HandleBroadcast(command.key, lbas);
    std::cout << "Write successful" << std::endl;
    std::cout << "Key: " << command.key << std::endl;
    //print out the LBAs
    for (int i = 0; i < 3; i++) {
        std::cout << "LBA " << i << ": " << lbas.lbas[i] << std::endl;
    }
    //TODO: Reply to the client
    return "Success";
}

std::string Gateway::HandleRead(KVRequest command) {
    // assume no failure, read from arbitrary logger
    int random = rand() % known_loggers.size();
    logger logger = known_loggers[random];
        rpc::client rc(logger.ip, logger.rport);
        cmd cmnd;
        cmnd.op = 1;
        cmnd.key = command.key;
        auto reply = rc.call("Read", cmnd, K_LBAs[command.key].lbas[random]);
        ReadReply read_reply = reply.as<ReadReply>();
        std::string msg = read_reply.value;
        std::cout << "Read value: " << msg << std::endl;
        //TODO: Reply to the client
        return msg;
}

void Gateway::HandleBroadcast(key_t key, LBAs lbas) {
    for (auto peer : known_peers) {
        std::cout << "Broadcasting to peer: " << peer.ip << peer.bport << std::endl;
        rpc::client bc(peer.ip, peer.bport);
        bc.call("HandleCatchup", key, lbas);
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
    server_thread.join();
    catchup_thread.join();
    return 0;
}
//usage: ./gateway <gateway_id(0-2)>