#include <iostream>
#include <boost/array.hpp>
#include <asio.hpp>

using asio::ip::tcp;

void server() {
    try {
        asio::io_context io_context;
        
        // Create an acceptor to listen for incoming connections
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 9999));
        
        // Create a socket to represent the connection to the client
        tcp::socket socket(io_context);
        
        // Wait for a client to connect
        acceptor.accept(socket);
        
        // Receive data from the client
        asio::streambuf buffer;
        asio::read(socket, buffer);
        
        // Output the received data
        std::istream input(&buffer);
        std::string message;
        std::getline(input, message);
        std::cout << "Received: " << message << std::endl;
        
        // Respond to the client
        std::string response = "Hello from server!";
        boost::asio::write(socket, boost::asio::buffer(response));
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

void client() {
    try {
        boost::asio::io_context io_context; // Update the namespace
        
        // Create a socket to connect to the server
        tcp::socket socket(io_context);
        
        // Connect to the server
        socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 9999));
        
        // Send data to the server
        std::string message = "Hello from client!";
        boost::asio::write(socket, boost::asio::buffer(message));
        
        // Receive response from the server
        boost::asio::streambuf buffer;
        boost::asio::read(socket, buffer);
        
        // Output the response
        std::istream input(&buffer);
        std::string response;
        std::getline(input, response);
        std::cout << "Response from server: " << response << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main() {
    // Run the server in a separate thread
    std::thread server_thread(server);
    
    // Run the client
    client();
    
    // Join the server thread
    server_thread.join();
    
    return 0;
}
// compile with: g++ -std=c++11 -pthread tcp-utils.cpp -o tcp-utils