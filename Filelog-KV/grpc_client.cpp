#include <iostream>
#include <memory>
#include "kv_request.grpc.pb.h" // Generated gRPC header

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using kvrequest::KVRequest;
using kvrequest::KVRequestService;

class KVRequestClient {
public:
    KVRequestClient(std::shared_ptr<Channel> channel) : stub_(KVRequestService::NewStub(channel)) {}

    void SendRequest(const KVRequest& request) {
        // Create a context for the RPC call
        ClientContext context;

        // Call the RPC method
        KVRequest response;
        Status status = stub_->ProcessRequest(&context, request, &response);

        // Check for errors
        if (status.ok()) {
            std::cout << "Request sent successfully" << std::endl;
        } else {
            std::cerr << "RPC failed: " << status.error_code() << ": " << status.error_message() << std::endl;
        }
    }

private:
    std::unique_ptr<KVRequestService::Stub> stub_;
};

int main() {
    // Create a gRPC channel
    std::shared_ptr<Channel> channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    KVRequestClient client(channel);

    // Create and send a KVRequest
    KVRequest request;
    request.set_request_type(KVRequest::GET);
    request.set_key(123);
    request.set_value("example value");

    client.SendRequest(request);

    return 0;
}

