#include "logger.hpp"

Logger::Logger() {
    // Constructor implementation goes here
}

Logger::~Logger() {
    // Destructor implementation goes here
}

int main() {
    Logger logger;

    // TODO: dispatch below on a separate threads
    rpc::server asrv(1111);
    asrv.bind("Append", []() {

    });
    asrv.run();

    rpc::server rsrv(2222);
    rsrv.bind("Read", []() {

    });

    return 0;
}