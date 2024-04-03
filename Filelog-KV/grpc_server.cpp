#include <iostream>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "kv_request.grpc.pb.h" // Generated gRPC header

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using kvrequest::KVRequest;
using kvrequest::KVRequestService;

class KVRequestServiceImpl final : public KVRequestService::Service {
    Status ProcessRequest(ServerContext* context, const KVRequest* request,
                          KVRequest* response) override {
        // Process the received KVRequest
        // For example, print the received data
        std::cout << "Received KVRequest:\n"
                  << "Request Type: " << request->request_type() << "\n"
                  << "Key: " << request->key() << "\n"
                  << "Value: " << request->value() << std::endl;

        // Return a response if needed
        return Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    KVRequestServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}

