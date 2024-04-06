#include "logger.hpp"

Logger::Logger() {
    // Constructor implementation goes here
}

Logger::~Logger() {
    // Destructor implementation goes here
}

// Function to convert std::string to byte buffer
void stringToBuffer(const std::string& serializedData, char* buffer, size_t bufferSize) {
    if (serializedData.size() > bufferSize) {
        // Handle error (buffer overflow)
        return;
    }
    memcpy(buffer, serializedData.data(), serializedData.size());
}

// Function to deserialize byte buffer into cmd struct
void DeserializeCMDRequest(const char* buffer, size_t bufferSize, cmd& request) {
    // Check if the buffer is large enough to hold the serialized data
    if (bufferSize < sizeof(CmdType) + sizeof(key_t) + BLOCK_SIZE) {
        // Handle error (e.g., throw an exception or return an error code)
        return;
    }

    // Deserialize op
    memcpy(&request.op, buffer, sizeof(CmdType));
    buffer += sizeof(CmdType);

    // Deserialize key
    memcpy(&request.key, buffer, sizeof(key_t));
    buffer += sizeof(key_t);

    // Deserialize value
    memcpy(request.value, buffer, BLOCK_SIZE);
}

void ProcessAppend(const std::string& serializedData) {
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(CmdType) + sizeof(key_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    cmd cmnd;
    DeserializeCMDRequest(buffer, bufferSize, cmnd);
    std::cout << cmnd.op << std::endl;
}

void ProcessRead(const std::string& serializedData) {
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(CmdType) + sizeof(key_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    cmd cmnd;
    DeserializeCMDRequest(buffer, bufferSize, cmnd);
    std::cout << cmnd.op << std::endl;
}


int main() {
    Logger logger;

    // TODO: dispatch below on a separate threads
   
    std::thread append_thread([]() { 
        rpc::server asrv(1111);
        asrv.bind("Append", [](std::string buffer) {
            ProcessAppend(buffer);
        });
        asrv.run();
    });

     std::thread read_thread([]() {
        rpc::server rsrv(2222);
        rsrv.bind("Read", [](std::string buffer) {
            ProcessRead(buffer);
            return 5;
        });
        rsrv.run();
    });


    append_thread.join();
    read_thread.join();

    return 0;
}
