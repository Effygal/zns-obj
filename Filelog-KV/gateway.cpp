#include "gateway.hpp"


// Function to serialize cmd struct into a byte buffer
void SerializeCMDRequest(const cmd& request, char* buffer, size_t bufferSize) {
    // Check if the buffer is large enough to hold the serialized data
    if (bufferSize < sizeof(CmdType) + sizeof(key_t) + BLOCK_SIZE) {
        // Handle error (e.g., throw an exception or return an error code)
        return;
    }

    // Serialize op
    memcpy(buffer, &request.op, sizeof(CmdType));
    buffer += sizeof(CmdType);

    // Serialize key
    memcpy(buffer, &request.key, sizeof(key_t));
    buffer += sizeof(key_t);

    // Serialize value
    memcpy(buffer, request.value, BLOCK_SIZE);
}

// Function to convert byte buffer to std::string
std::string bufferToString(const char* buffer, size_t bufferSize)
{
    return std::string(buffer, buffer + bufferSize);
}

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
void DeserializeKVRequest(const char* buffer, size_t bufferSize, KVRequest& request)
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

void TranslateKVReq(KVRequest request, cmd& cmnd) {
    if (request.request_type == 1) {
        cmnd.op = READ;
        cmnd.key = request.key;
    }
    else if (request.request_type == 2) {
        cmnd.op = APPEND;
        cmnd.key = request.key;
        std::memcpy(cmnd.value, request.value, BLOCK_SIZE);
    }
    else if (request.request_type == 3) {
        // Fill in late if necessary
    }
}

void ProcessGetRequest(const std::string& serializedData)
{
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    // Deserialize the byte buffer into a KVRequest struct
    KVRequest request;
    DeserializeKVRequest(buffer, bufferSize, request);

    // Process GET request here

}

void ProcessPutRequest(const std::string& serializedData)
{
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    // Deserialize the byte buffer into a KVRequest struct
    KVRequest request;
    DeserializeKVRequest(buffer, bufferSize, request);

    // Process PUT request here
    cmd translated_cmd;
    TranslateKVReq(request, translated_cmd);
    // Serialize the request
    constexpr size_t cmdbufferSize = sizeof(CmdType) + sizeof(key_t) + BLOCK_SIZE;
    char cmdbuffer[cmdbufferSize];
    SerializeCMDRequest(translated_cmd, cmdbuffer, cmdbufferSize);
    std::string serializedCMD = bufferToString(cmdbuffer, cmdbufferSize);
    rpc::client logger("127.0.0.1", 1111);	    
    logger.call("Append", serializedCMD);
}

void ProcessDelRequest(const std::string& serializedData)
{
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(RequestType) + sizeof(int32_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    // Deserialize the byte buffer into a KVRequest struct
    KVRequest request;
    DeserializeKVRequest(buffer, bufferSize, request);

    // Process DELETE request here

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


    read_thread.join();
    write_thread.join();
    del_thread.join();
    catchup_thread.join();

    return 0;
}

