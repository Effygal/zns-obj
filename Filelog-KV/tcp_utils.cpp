#include "tcp_utils.hpp"
#include <iostream>
#include <asio.hpp>
#include <string>

using namespace std;
using namespace asio::ip;

void tcp_server(unsigned short port) {
    try {
        asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            cout << "New connection from: " << socket.remote_endpoint().address().to_string() << endl;

            asio::streambuf buffer;
            asio::read_until(socket, buffer, '\n');

            string data = asio::buffer_cast<const char*>(buffer.data());
            cout << "Received: " << data << endl;

            asio::write(socket, asio::buffer(data));
        }
    } catch (exception& e) {
        cerr << "Exception in server: " << e.what() << endl;
    }
}

void tcp_client(const string& host, unsigned short port, const string& message) {
    try {
        asio::io_context io_context;

        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(asio::ip::address::from_string(host), port));

        asio::write(socket, asio::buffer(message));

        asio::streambuf buffer;
        asio::read_until(socket, buffer, '\n');

        string response = asio::buffer_cast<const char*>(buffer.data());
        cout << "Response: " << response << endl;
    } catch (exception& e) {
        cerr << "Exception in client: " << e.what() << endl;
    }
}
