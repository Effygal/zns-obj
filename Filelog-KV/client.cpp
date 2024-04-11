#include "client.hpp"
#include "read_config.hpp"

bool parseCommand(const std::string& command, KVRequest& request) {
    std::istringstream iss(command);
    std::string token;

    // Parse request type
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

    // Parse key
    if (!(iss >> token)) {
        std::cerr << "Error: Missing key." << std::endl;
        return false;
    }
    request.key = std::stoi(token);

    // Parse value for PUT command
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

    // Check if there are any extra tokens
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
        // Get user input for command
        std::getline(std::cin, command);

        // Parse command and extract request type, key, and value
        if (!parseCommand(command, request)) {
            continue; // Ask for input again if parsing fails
        }
        // Send the request to the gateway
        // send_request_to_gateway(request, result.first, result.second);
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

//usage: ./client