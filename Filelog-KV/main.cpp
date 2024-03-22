#include "tcp_utils.hpp"
#include "udp_utils.hpp"
#include <thread>
#include <chrono>
#include <string>
#include <iostream>

int main() {
    unsigned short tcp_port = 3333;
    unsigned short udp_port = 4444;
    std::string host = "127.0.0.1";
    std::string message = "Hello, world!\n";

    // Test TCP
    std::thread tcp_server_thread(tcp_server, tcp_port);
    std::this_thread::sleep_for(std::chrono::seconds(1)); 
    tcp_client(host, tcp_port, message);
    tcp_server_thread.join();

    // Test UDP
    std::thread udp_server_thread(udp_server, udp_port);
    std::this_thread::sleep_for(std::chrono::seconds(1)); 
    udp_client(host, udp_port, message);
    udp_server_thread.join();

    return 0;
}

//usage: g++ main.cpp -std=c++11 tcp_utils.cpp udp_utils.cpp -o main -pthread -lboost_system -lboost_thread -lboost_chrono