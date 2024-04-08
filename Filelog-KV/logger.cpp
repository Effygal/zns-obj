#include "logger.hpp"

class Logger {

	public:
		Logger() : _cur_lba(0) {
            pthread_mutex_init(&_mutex, nullptr);
        }

        ~Logger() {
            pthread_mutex_destroy(&_mutex);
        }
        off_t Append(const LogEnt& logent) {
            pthread_mutex_lock(&_mutex);
            pwrite(_fd, &logent, sizeof(LogEnt), _cur_lba);
            off_t offset = lseek(_fd, sizeof(LogEnt), SEEK_END);
            _cur_lba = offset;
            pthread_mutex_unlock(&_mutex);
            ReplyAppend(logent.metadata.key, offset);
            return offset;
        }
        void Read(key_t key, void* buff) {
            off_t lba = K_LBAs[key].lbas[_logger_id] - sizeof(LogEnt);
            pread(_fd, buff, sizeof(LogEnt), lba);
        }

        void ReplyAppend(key_t key, off_t lba) {
            //only reply to the gateway that sent the request


        }   
        void ReplyRead(void* buff){
            //only reply to the gateway that sent the request

        }

        void AppendThread(cmd& request) {
            LogEnt logent(request.key, request.value);
            off_t lba = Append(logent);
            ReplyAppend(request.key, lba);
            
        }
        void ReadThread(cmd& request) {
            
        }

		private:
			ssize_t _len;
			int _fd;
			off_t _cur_lba;
			pthread_mutex_t _mutex;
            int16_t _logger_id;
            std::vector<std::pair<std::string, int>> _gws; // gateway servers, <ip, port>
            int _port;
};

// Function to convert std::string to byte buffer
void stringToBuffer(const std::string& serializedData, char* buffer, size_t bufferSize) {
    if (serializedData.size() > bufferSize) {
        // Handle error (buffer overflow)
        return;
    }
    memcpy(buffer, serializedData.data(), serializedData.size());
}

// Function to deserialize byte buffer into cmd struct
void DeserializeCMDRequest(const char* buffer, size_t bufferSize, cmd& request) {
    // Check if the buffer is large enough to hold the serialized data
    if (bufferSize < sizeof(CmdType) + sizeof(key_t) + BLOCK_SIZE) {
        // Handle error (e.g., throw an exception or return an error code)
        return;
    }

    // Deserialize op
    memcpy(&request.op, buffer, sizeof(CmdType));
    buffer += sizeof(CmdType);

    // Deserialize key
    memcpy(&request.key, buffer, sizeof(key_t));
    buffer += sizeof(key_t);

    // Deserialize value
    memcpy(request.value, buffer, BLOCK_SIZE);
}

void ProcessAppend(const std::string& serializedData) {
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(CmdType) + sizeof(key_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    cmd cmnd;
    DeserializeCMDRequest(buffer, bufferSize, cmnd);
    std::cout << cmnd.op << std::endl;
}

void ProcessRead(const std::string& serializedData) {
    // Convert the received std::string to a byte buffer
    constexpr size_t bufferSize = sizeof(CmdType) + sizeof(key_t) + BLOCK_SIZE;
    char buffer[bufferSize];
    stringToBuffer(serializedData, buffer, bufferSize);

    cmd cmnd;
    DeserializeCMDRequest(buffer, bufferSize, cmnd);
    std::cout << cmnd.op << std::endl;
}


int main() {
    Logger logger;

    // TODO: dispatch below on a separate threads
   
    std::thread append_thread([]() { 
        rpc::server asrv(4444);
        asrv.bind("Append", [](std::string buffer) {
            ProcessAppend(buffer);
            // RETRURN VALUE NEEDS TO CHANGE LATER (TO THE LBA OR THE VALUE BASED ON REQ TYPE)
            return 1;
        });
        asrv.run();
    });

     std::thread read_thread([]() {
        rpc::server rsrv(19999);
        rsrv.bind("Read", [](std::string buffer) {
            ProcessRead(buffer);
            return 1;
        });
        rsrv.run();
    });


    append_thread.join();
    read_thread.join();

    return 0;
}
