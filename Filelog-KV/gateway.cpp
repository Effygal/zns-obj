#include "gateway.hpp"


// Function to convert std::string to byte buffer
void stringToBuffer(const std::string& serializedData, char* buffer, size_t bufferSize)
{
    if (serializedData.size() > bufferSize) {
        // Handle error (buffer overflow)
        return;
    }
    memcpy(buffer, serializedData.data(), serializedData.size());
}

// Function to deserialize byte buffer into KVRequest struct
void deserializeKVRequest(const char* buffer, size_t bufferSize, KVRequest& request)
{
    // Check if the buffer is large enough to hold the serialized data
    if (bufferSize < sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE) {
        // Handle error (e.g., throw an exception or return an error code)
        return;
    }

    // Deserialize request_type
    memcpy(&request.request_type, buffer, sizeof(RequestType));
    buffer += sizeof(RequestType);

    // Deserialize key
    memcpy(&request.key, buffer, sizeof(int32_t));
    buffer += sizeof(int32_t);

    // Deserialize value
    memcpy(request.value, buffer, BLOCK_SIZE);
}


void ProcessGetRequest(const std::string& serializedData)
{
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    // Deserialize the byte buffer into a KVRequest struct
    KVRequest request;
    deserializeKVRequest(buffer, bufferSize, request);

    // Now 'request' contains the deserialized data and can be processed accordingly
    std::cout << "Command: " << request.request_type << std::endl;
}

void ProcessPutRequest(const std::string& serializedData)
{
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    // Deserialize the byte buffer into a KVRequest struct
    KVRequest request;
    deserializeKVRequest(buffer, bufferSize, request);

    // Now 'request' contains the deserialized data and can be processed accordingly
    std::cout << "Command: " << request.request_type << std::endl;
}

void ProcessDelRequest(const std::string& serializedData)
{
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    // Deserialize the byte buffer into a KVRequest struct
    KVRequest request;
    deserializeKVRequest(buffer, bufferSize, request);

    // Now 'request' contains the deserialized data and can be processed accordingly
    std::cout << "Command: " << request.request_type << std::endl;
}


int main() {
    

    std::thread read_thread([]() {
        rpc::server rsrv(5555);
        rsrv.bind("HandleRead", [](std::string buffer) {
		ProcessGetRequest(buffer);
	});
        rsrv.run();
    });

    std::thread write_thread([]() {
        rpc::server rsrv(6666);
        rsrv.bind("HandleWrite", [](std::string buffer) {
		ProcessPutRequest(buffer);
	});
        rsrv.run();
    });

    std::thread del_thread([]() {
        rpc::server rsrv(7777);
        rsrv.bind("HandleDel", [](std::string buffer) {
		ProcessDelRequest(buffer);
	});
        rsrv.run();
    });

    std::thread catchup_thread([]() {
        rpc::server csrv(8888);
        csrv.bind("CatchupMeeple", []() {
        });
        csrv.run();
    });

    constexpr int num_write_threads = 4; // Adjust this as needed
    std::vector<std::thread> write_thread_pool;
    /*for (int i = 0; i < num_write_threads; ++i) {
        write_thread_pool.emplace_back([]() {
            rpc::server wsrv(6666);
            wsrv.bind("HandleWrite", [](int a, int b) {
                return a + b;
            });
            wsrv.run();
        });
    }*/

    read_thread.join();
    for (auto& thread : write_thread_pool) {
        thread.join();
    }

    return 0;
}

