#include "logger.hpp"

off_t Logger::Append(const LogEnt& logent) {
    pthread_mutex_lock(&_mutex);
    pwrite(_fd, &logent, sizeof(LogEnt), _cur_lba);
    off_t offset = lseek(_fd, sizeof(LogEnt), SEEK_END);
    _cur_lba = offset;
    pthread_mutex_unlock(&_mutex);
    return offset;
}
void Logger::Read(key_t key, void* buff, off_t lba) {
    pread(_fd, buff, sizeof(LogEnt), lba);
}

// void Logger::ReplyAppend(key_t key, off_t lba, int8_t gw_id) {
//     //only reply to the gateway that sent the request
//     rpc::client c(_gws[gw_id].first, _gws[gw_id].second);

// }   
// void Logger::ReplyRead(void* buff){
//     //only reply to the gateway that sent the request

// }

AppendReply Logger::AppendThread(cmd& request) {
    std::cout << "Received Append request "<< request.key << request.value << std::endl;

    LogEnt logent(request.key, request.value);
    off_t lba = Append(logent);
    AppendReply reply = {_logger_id, request.key, lba};
    return reply;
}
ReadReply Logger::ReadThread(cmd& request, off_t lba) {
    Read(request.key, request.value, lba);
    std::string value(request.value);
    ReadReply reply = {request.key, value};
    return reply;
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
    if (bufferSize < sizeof(int8_t) + sizeof(key_t) + BLOCK_SIZE) {
        // Handle error (e.g., throw an exception or return an error code)
        return;
    }

    // Deserialize op
    memcpy(&request.op, buffer, sizeof(int8_t));
    buffer += sizeof(int8_t);

    // Deserialize key
    memcpy(&request.key, buffer, sizeof(key_t));
    buffer += sizeof(key_t);

    // Deserialize value
    memcpy(request.value, buffer, BLOCK_SIZE);
}

void ProcessAppend(const std::string& serializedData) {
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(int8_t) + sizeof(key_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    cmd cmnd;
    DeserializeCMDRequest(buffer, bufferSize, cmnd);
    std::cout << cmnd.op << std::endl;
}

void ProcessRead(const std::string& serializedData) {
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(int8_t) + sizeof(key_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    cmd cmnd;
    DeserializeCMDRequest(buffer, bufferSize, cmnd);
    std::cout << cmnd.op << std::endl;
    
}


int main(int argc, char** argv) {
    int8_t loggerID = atoi(argv[1]);//0-2
    Logger logger; 
    logger._logger_id = loggerID; //dup this info to config file later
    logger.wport = 6000 + loggerID;
    logger.rport = 5000 + loggerID;
    // std::thread append_thread([&logger]() { 
    //     rpc::server asrv(logger.wport);
    //     asrv.bind("Append", [&logger](cmd request) -> AppendReply {
    //         std::cout << "Received Append request "<< request.key << request.value << std::endl;
    //         AppendReply reply = logger.AppendThread(request);
    //         return reply;//client on gateway get this reply
    //     });
    //     asrv.run();
    // });

    std::thread append_thread([&logger]() { 
        rpc::server asrv(logger.wport);
        asrv.bind("Append", [&logger](cmd request) -> AppendReply {
            std::cout << "Received Append request "<< request.key << request.value << std::endl;
            AppendReply reply = logger.AppendThread(request);
            return reply;//client on gateway get this reply
        });
        asrv.run();
    });

    std::thread read_thread([&logger]() {
        rpc::server rsv(logger.rport);
        rsv.bind("Read", [&logger](cmd request, off_t lba) -> ReadReply {
            ReadReply reply = logger.ReadThread(request, lba);
            return reply;
        });
        rsv.run();
    });

    append_thread.join();
    read_thread.join();
    return 0;
}
//usage: ./logger <logger_id(0-2)>