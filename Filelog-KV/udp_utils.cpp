#include "udp_utils.hpp"
#include <iostream>
#include <asio.hpp>
#include <string>
#include <array>
using namespace std;
using namespace asio::ip;

void udp_server(unsigned short port) {
    try {
        asio::io_context io_context;

        udp::socket socket(io_context, udp::endpoint(udp::v4(), port));

        while (true) {
            array<char, 1024> recv_buffer;
            udp::endpoint remote_endpoint;
            size_t len = socket.receive_from(asio::buffer(recv_buffer), remote_endpoint);

            string data(recv_buffer.data(), len);
            cout << "Received from " << remote_endpoint.address().to_string() << ": " << data << endl;

            socket.send_to(asio::buffer(data), remote_endpoint);
        }
    } catch (exception& e) {
        cerr << "Exception in server: " << e.what() << endl;
    }
}

void udp_client(const string& host, unsigned short port, const string& message) {
    try {
        asio::io_context io_context;

        udp::socket socket(io_context, udp::endpoint(udp::v4(), 0));

        udp::resolver resolver(io_context);
        udp::endpoint receiver_endpoint = *resolver.resolve(udp::v4(), host, to_string(port)).begin();

        socket.send_to(asio::buffer(message), receiver_endpoint);

        array<char, 1024> recv_buffer;
        udp::endpoint sender_endpoint;
        size_t len = socket.receive_from(asio::buffer(recv_buffer), sender_endpoint);

        string response(recv_buffer.data(), len);
        cout << "Response from " << sender_endpoint.address().to_string() << ": " << response << endl;
    } catch (exception& e) {
        cerr << "Exception in client: " << e.what() << endl;
    }
}
