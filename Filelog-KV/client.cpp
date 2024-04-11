/*
    ------------------------------------
    Client.cpp
    ------------------------------------
    Usage: ./client
    PUT <key> <value>
    GET <key>
*/

#include "client.hpp"
#include "read_config.hpp"

bool parseCommand(const std::string& command, KVRequest& request) {
    std::istringstream iss(command);
    std::string token;

    if (!(iss >> token)) {
        std::cerr << "Error: Missing request type." << std::endl;
        return false;
    }
    if (token == "PUT") {
        request.request_type = 2;
    } else if (token == "GET") {
        request.request_type = 1;
    } else if (token == "DEL") {
        request.request_type = 3;
    } else {
        std::cerr << "Error: Invalid request type." << std::endl;
        return false;
    }

    if (!(iss >> token)) {
        std::cerr << "Error: Missing key." << std::endl;
        return false;
    }
    request.key = std::stoi(token);

    if (request.request_type == 2) {
        if (!(iss >> token)) {
            std::cerr << "Error: Missing value for PUT command." << std::endl;
            return false;
        }
        if (token.size() >= BLOCK_SIZE) {
            std::cerr << "Error: Value exceeds maximum size." << std::endl;
            return false;
        }
        memcpy(request.value, token.c_str(), token.size() + 1); // +1 to include null terminator
    }

    if (iss >> token) {
        std::cerr << "Error: Unexpected token '" << token << "'." << std::endl;
        return false;
    }

    return true;
}

int main() {
    std::string command;
    KVRequest request;
    std::string reply;
    // Read config.json file
    // Config conf = parseConfig("config.json");

    // Split IP address and port number
    // auto result = splitAddress(conf.gateways[ChooseRandGateway(0, conf.gateways.size() - 1)]);

    while (true) {
        std::getline(std::cin, command);

        if (!parseCommand(command, request)) {
            continue;
        }
        //try this:
        //choose a random gateway
        int rand = random() % 3;
        int gateway_cport = 6600 + rand;
        rpc::client client("127.0.0.1", gateway_cport);
        std::cout << "Connected to gateway "<< gateway_cport << std::endl;
        switch(request.request_type) {
            case 2:
                reply = client.call("HandleWrite", request).as<std::string>();
                std::cout << reply << std::endl;
                break;
            case 1:
                reply = client.call("HandleRead", request).as<std::string>();
                std::cout << "Read value: " << reply << std::endl;
                break;
            default:
                break;
        }
    } 

    return 0;
}
