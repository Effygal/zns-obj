#include "gateway.hpp"

// Choose a random number based on uniform distribution
int ChooseRandLogger(int min, int max) {
    std::random_device rd;
    std::mt19937 gen(rd());

    // Create a uniform distribution
    std::uniform_int_distribution<> distr(min, max);

    return distr(gen);
}

// Make a tuple of IP address and ports
std::tuple<std::string, int, int> parseAddress(const std::string& addressStr) {
    // Find the position of the colon
    size_t colonPos = addressStr.find(':');
    if (colonPos == std::string::npos) {
        throw std::invalid_argument("Invalid address format: missing colon");
    }

    // Extract the IP address and port numbers
    std::string ip = addressStr.substr(0, colonPos);
    size_t commaPos = addressStr.find(',');
    if (commaPos == std::string::npos) {
        throw std::invalid_argument("Invalid address format: missing comma");
    }
    int port1 = std::stoi(addressStr.substr(colonPos + 1, commaPos - colonPos - 1));
    int port2 = std::stoi(addressStr.substr(commaPos + 2)); // Skip comma and space

    return std::make_tuple(ip, port1, port2);
}

// Function to serialize cmd struct into a byte buffer
void SerializeCMDRequest(const cmd& request, char* buffer, size_t bufferSize) {
    // Check if the buffer is large enough to hold the serialized data
    if (bufferSize < sizeof(CmdType) + sizeof(key_t) + BLOCK_SIZE) {
        // Handle error (e.g., throw an exception or return an error code)
        return;
    }

    // Serialize op
    memcpy(buffer, &request.op, sizeof(CmdType));
    buffer += sizeof(CmdType);

    // Serialize key
    memcpy(buffer, &request.key, sizeof(key_t));
    buffer += sizeof(key_t);

    // Serialize value
    memcpy(buffer, request.value, BLOCK_SIZE);
}

// Function to convert byte buffer to std::string
std::string bufferToString(const char* buffer, size_t bufferSize)
{
    return std::string(buffer, buffer + bufferSize);
}

// Function to convert std::string to byte buffer
void stringToBuffer(const std::string& serializedData, char* buffer, size_t bufferSize)
{
    if (serializedData.size() > bufferSize) {
        // Handle error (buffer overflow)
        return;
    }
    memcpy(buffer, serializedData.data(), serializedData.size());
}

// Function to deserialize byte buffer into KVRequest struct
void DeserializeKVRequest(const char* buffer, size_t bufferSize, KVRequest& request)
{
    // Check if the buffer is large enough to hold the serialized data
    if (bufferSize < sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE) {
        // Handle error (e.g., throw an exception or return an error code)
        return;
    }

    // Deserialize request_type
    memcpy(&request.request_type, buffer, sizeof(RequestType));
    buffer += sizeof(RequestType);

    // Deserialize key
    memcpy(&request.key, buffer, sizeof(int32_t));
    buffer += sizeof(int32_t);

    // Deserialize value
    memcpy(request.value, buffer, BLOCK_SIZE);
}

void TranslateKVReq(KVRequest request, cmd& cmnd) {
    if (request.request_type == 1) {
        cmnd.op = READ;
        cmnd.key = request.key;
    }
    else if (request.request_type == 2) {
        cmnd.op = APPEND;
        cmnd.key = request.key;
    }
    else if (request.request_type == 3) {
        // Fill in late if necessary
    }
}

// void ProcessGetRequest(const std::string& serializedData)
// {
//     // Convert the received std::string to a byte buffer
//     constexpr size_t bufferSize = sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE;
//     char buffer[bufferSize];
//     stringToBuffer(serializedData, buffer, bufferSize);

//     // Deserialize the byte buffer into a KVRequest struct
//     KVRequest request;
//     DeserializeKVRequest(buffer, bufferSize, request);

//     // Process GET request here
//     cmd translated_cmd;
//     TranslateKVReq(request, translated_cmd);
//     // Serialize the request
//     constexpr size_t cmdbufferSize = sizeof(CmdType) + sizeof(key_t) + BLOCK_SIZE;
//     char cmdbuffer[cmdbufferSize];
//     SerializeCMDRequest(translated_cmd, cmdbuffer, cmdbufferSize);
//     std::string serializedCMD = bufferToString(cmdbuffer, cmdbufferSize);
//     rpc::client logger("127.0.0.1", 2222);	    
//     auto result = logger.call("Read", serializedCMD).as<int>();
// }

void ProcessRequest(const std::string& serializedData, Config conf)
{
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    // Deserialize the byte buffer into a KVRequest struct
    KVRequest request;
    DeserializeKVRequest(buffer, bufferSize, request);

    // Process PUT request here
    cmd translated_cmd;
    std::cout << request.request_type << std::endl;
    std::cout << request.key << std::endl;
    std::cout << request.value << std::endl;
    TranslateKVReq(request, translated_cmd);
    // Serialize the request
    constexpr size_t cmdbufferSize = sizeof(CmdType) + sizeof(key_t) + BLOCK_SIZE;
    char cmdbuffer[cmdbufferSize];
    SerializeCMDRequest(translated_cmd, cmdbuffer, cmdbufferSize);
    std::string serializedCMD = bufferToString(cmdbuffer, cmdbufferSize);

    if (request.request_type == 1) {
        // ON READS, WE NEED OT CONSULT THE MAP TO SEE WHERE IT IS
        // std::string ip;
        // // r for read and a for append
        // int rport, aport;
        // std::tie(ip, rport, aport) = parseAddress(conf.loggers[FROM MAP]);
        // rpc::client logger(ip, rport);	
        //rpc::client logger(ip, rport);	
        //auto result = logger.call("Read", serializedCMD).as<int>();
    }   
    else if (request.request_type == 2) {
        // ON WRITES, WE WILL JUST CHOOSE A RANDOM LOGGER TO WRITE INTO
        std::string ip;
        // r for read and a for append
        int rport, aport;
        std::tie(ip, rport, aport) = parseAddress(conf.loggers[ChooseRandLogger(0, conf.loggers.size() - 1)]);
        rpc::client logger(ip, aport);	
        auto result = logger.call("Append", serializedCMD).as<int>();
    }
}

// void ProcessDelRequest(const std::string& serializedData)
// {
//     // Convert the received std::string to a byte buffer
//     constexpr size_t bufferSize = sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE;
//     char buffer[bufferSize];
//     stringToBuffer(serializedData, buffer, bufferSize);

//     // Deserialize the byte buffer into a KVRequest struct
//     KVRequest request;
//     DeserializeKVRequest(buffer, bufferSize, request);

//     // Process DELETE request here

// }


int main() {

    // Read config.json file
    Config conf = parseConfig("config.json");
    
    std::thread req_thread([conf]() {
        rpc::server rsrv(1111);
        rsrv.bind("HandleReq", [conf](std::string buffer) {
		ProcessRequest(buffer, conf);
	});
        rsrv.run();
    });

    /*std::thread write_thread([]() {
        rpc::server rsrv(6666);
        rsrv.bind("HandleWrite", [](std::string buffer) {
		ProcessPutRequest(buffer);
	});
        rsrv.run();
    });

    std::thread del_thread([]() {
        rpc::server rsrv(7777);
        rsrv.bind("HandleDel", [](std::string buffer) {
		ProcessDelRequest(buffer);
	});
        rsrv.run();
    });*/

    std::thread catchup_thread([]() {
        rpc::server csrv(8888);
        csrv.bind("CatchupMeeple", []() {
        });
        csrv.run();
    });


    req_thread.join();
    // write_thread.join();
    // del_thread.join();
    catchup_thread.join();

    return 0;
}

