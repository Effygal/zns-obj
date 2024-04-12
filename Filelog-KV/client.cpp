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
    if (token == "put" || token == "PUT") {
        request.request_type = 2;
    } else if (token == "get" || token == "GET") {
        request.request_type = 1;
    } else if (token == "del" || token == "DEL") {
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
    Config conf = parseConfig("config.json");

    while (true) {
        std::getline(std::cin, command);

        if (!parseCommand(command, request)) {
            continue;
        }
        int num_gateways = conf.gateways.size(); // how many gateways?
        std::string gateway_ip;
        int gateway_cport;

        while (true) {
            int rand = random() % num_gateways;
            auto result = parseAddress(conf.gateways[rand]);
            // Access elements of the returned tuple
            gateway_ip = std::get<0>(result);
            gateway_cport = std::get<2>(result);

            try {
                // Attempt to connect to the gateway
                rpc::client client(gateway_ip, gateway_cport);
                std::cout << "Connected to gateway " << gateway_cport << std::endl;
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
                break;
            } catch (const std::exception& e) {
                // If connection fails, print error message and try the next gateway
                std::cerr << "Error: " << e.what() << std::endl;
                std::cout << "Failed to connect to gateway " << gateway_cport << ". Trying next gateway..." << std::endl;
                continue;
            }
        }
    } 
    return 0;
}