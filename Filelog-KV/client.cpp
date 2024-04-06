#include "client.hpp"
#include "read_config.hpp"

// Choose a random number based on uniform distribution
int ChooseRandGateway(int min, int max) {
    std::random_device rd;
    std::mt19937 gen(rd());

    // Create a uniform distribution
    std::uniform_int_distribution<> distr(min, max);

    return distr(gen);
}

// Make a pair of IP address and port
std::pair<std::string, int> splitAddress(const std::string& address) {
    std::istringstream iss(address);
    std::string ip, portStr;

    // Get IP address
    std::getline(iss, ip, ':');

    // Get port number
    std::getline(iss, portStr);

    // Convert port string to integer
    int port = std::stoi(portStr);

    return std::make_pair(ip, port);
}

// Serializing the request in order to send it over the network
void serializeKVRequest(const KVRequest& request, char* buffer, size_t bufferSize)
{
    // Check if the buffer is large enough to hold the serialized data
    if (bufferSize < sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE) {
        // Handle error (e.g., throw an exception or return an error code)
        return;
    }

    // Serialize request_type
    memcpy(buffer, &request.request_type, sizeof(RequestType));
    buffer += sizeof(RequestType);

    // Serialize key
    memcpy(buffer, &request.key, sizeof(int32_t));
    buffer += sizeof(int32_t);

    // Serialize value
    memcpy(buffer, request.value, BLOCK_SIZE);
}

// Function to convert byte buffer to std::string
std::string bufferToString(const char* buffer, size_t bufferSize)
{
    return std::string(buffer, buffer + bufferSize);
}

// Define the RPC client
void send_request_to_gateway(const KVRequest& request, std::string ip, int port) {
    // Serialize the request
    constexpr size_t bufferSize = sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    serializeKVRequest(request, buffer, bufferSize);
    // Convert the byte buffer to std::string
    std::string serializedData = bufferToString(buffer, bufferSize);

    rpc::client client(ip, port);	    
    // Send the serialized request to the gateway
    client.call("HandleReq", serializedData);
}

// Function to parse command and extract request type, key, and value
bool parseCommand(const std::string& command, KVRequest& request)
{
    // Convert the command string to uppercase for case-insensitive comparison
    std::string upperCommand = command;
    std::transform(upperCommand.begin(), upperCommand.end(), upperCommand.begin(), ::toupper);

    // Find the position of the opening parenthesis
    auto openParenPos = upperCommand.find('(');

    // Find the position of the closing parenthesis
    auto closeParenPos = upperCommand.find(')');

    // Check if parentheses are found
    if (openParenPos != std::string::npos && closeParenPos != std::string::npos) {
        // Extract the command type (GET, PUT, DEL)
        std::string commandType = upperCommand.substr(0, openParenPos);

        // Set the request type based on the command type
        if (commandType == "GET") {
            request.request_type = GET;
            // Check if there's anything after the closing parenthesis
            if (upperCommand.substr(closeParenPos + 1).find_first_not_of(" \t\n\v\f\r") != std::string::npos) {
                std::cerr << "GET command should have only one argument (the key)\n";
                return false;
            }
            // Check if there are any commas between the arguments
            if (std::count(upperCommand.begin(), upperCommand.end(), ',') > 0) {
                std::cerr << "GET command should have only one argument (the key)\n";
                return false;
            }
        } else if (commandType == "PUT") {
            request.request_type = PUT;
            // Check if there's exactly one comma between the arguments
            if (std::count(upperCommand.begin(), upperCommand.end(), ',') != 1) {
                std::cerr << "PUT command should have two arguments (the key and the value)\n";
                return false;
            }
        } else if (commandType == "DEL") {
            request.request_type = DEL;
            // Check if there's anything after the closing parenthesis
            if (upperCommand.substr(closeParenPos + 1).find_first_not_of(" \t\n\v\f\r") != std::string::npos) {
                std::cerr << "DEL command should have only one argument (the key)\n";
                return false;
            }
            // Check if there are any commas between the arguments
            if (std::count(upperCommand.begin(), upperCommand.end(), ',') > 0) {
                std::cerr << "DEL command should have only one argument (the key)\n";
                return false;
            }
        } else {
            std::cerr << "Invalid command type\n";
            return false;
        }

        // Extract the key
        std::string keyStr = upperCommand.substr(openParenPos + 1, closeParenPos - openParenPos - 1);
        try {
            request.key = std::stoi(keyStr);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid key format\n";
            return false;
        } catch (const std::out_of_range& e) {
            std::cerr << "Key out of range\n";
            return false;
        }

        // If PUT or DEL command, check if there's any value specified
        if (request.request_type == PUT) {
            auto commaPos = upperCommand.find(',');
            if (commaPos == std::string::npos) {
                std::cerr << "Command should have two arguments (the key and the value)\n";
                return false;
            }
            const std::string valueSubstr = command.substr(upperCommand.find(",") + 2, upperCommand.find(")") - upperCommand.find(",") - 2);
            std::strncpy(request.value, valueSubstr.c_str(), BLOCK_SIZE - 1);
            request.value[BLOCK_SIZE - 1] = '\0'; // Ensure null-termination
        }

        return true;
    } else {
        std::cerr << "Invalid command format\n";
        return false;
    }
}


int main() {

    std::string command;
    KVRequest request;

    // Read config.json file
    Config conf = parseConfig("config.json");

    // Split IP address and port number
    auto result = splitAddress(conf.gateways[ChooseRandGateway(0, conf.gateways.size() - 1)]);

    while (true) {
        // Get user input for command
        std::getline(std::cin, command);

        // Parse command and extract request type, key, and value
        if (!parseCommand(command, request)) {
            continue; // Ask for input again if parsing fails
        }

        // Send the request to the gateway
        send_request_to_gateway(request, result.first, result.second);
    } 

    return 0;
}
