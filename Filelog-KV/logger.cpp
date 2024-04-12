/*
    ------------------------------------
    Loggers
    ------------------------------------
    Usage: ./logger <logger_id(0-2)>
*/
#include "logger.hpp"

off_t Logger::Append(const LogEnt& logent) {
    pthread_mutex_lock(&_mutex);
    ssize_t bytes_written = pwrite(_fd, &logent, sizeof(LogEnt), _cur_lba);
    if (bytes_written < 0) {
        perror("Write error");
        pthread_mutex_unlock(&_mutex);
        return -1;
    }
    off_t offset = lseek(_fd, bytes_written, SEEK_CUR); 
    if (offset < 0) {
        perror("Seek error");
        pthread_mutex_unlock(&_mutex);
        return -1; 
    }
    _cur_lba = offset;
    std::cout << "Current write pointer: " << _cur_lba << std::endl;
    pthread_mutex_unlock(&_mutex);
    return offset;
}

AppendReply Logger::AppendThread(cmd& request) {
    LogEnt logent(request.key, request.value);
    off_t lba = Append(logent);
    AppendReply reply = {_logger_id, request.key, lba};
    return reply;
}

ReadReply Logger::ReadThread(cmd& request, off_t lba) {
    char buffer[BLOCK_SIZE];
    ssize_t bytes_read = pread(_fd, buffer, sizeof(buffer), lba - BLOCK_SIZE);
    if (bytes_read < 0) {
        perror("Read error");
        return {}; 
    }
    std::cout << "Read " << bytes_read << " bytes" << std::endl;
    std::string value(buffer, bytes_read); 
    std::cout << "Read value: " << value << std::endl;
    ReadReply reply = {request.key, value};
    return reply;
}

int main(int argc, char** argv) {

    if (argc < 2 ) {
        std::cerr << "Usage: ./logger <logger_id(0-2)>" << std::endl;
        return 1;
    }

    // Read config.json file
    Config conf = parseConfig("config.json");
    
    int8_t loggerID = atoi(argv[1]);//0-2
    Logger logger; 
    logger._logger_id = loggerID; // read this from config file later...
    auto result = parseAddress(conf.loggers[logger._logger_id]);
    logger.rport = std::get<1>(result);
    logger.wport = std::get<2>(result);

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
