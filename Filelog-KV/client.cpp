/*
    ------------------------------------
    Client.cpp
    ------------------------------------
    Usage: ./client <type>
    PUT <key> <value>
    GET <key>
    ------------------------------------
    Measurement of latency and throughput under type 1:
    ./client 1 <trace_file>
    Results are written to statistics/latency.txt and statistics/throughput.txt
*/
#include <fstream>
#include "client.hpp"
#include "read_config.hpp"
#include <chrono>
#include <vector>
#include <algorithm>

struct RequestInfo {
    std::chrono::milliseconds latency;
    std::string reply;
};

bool parseCommand(const std::string& command, KVRequest& request) {
    std::istringstream iss(command);
    std::string token;

    if (!(iss >> token)) {
        std::cerr << "Error: Missing request type." << std::endl;
        return false;
    }
    if (token == "put" || token == "PUT") {
        request.request_type = 2;
    } else if (token == "get" || token == "GET") {
        request.request_type = 1;
    } else if (token == "del" || token == "DEL") {
        request.request_type = 3;
    } else {
        std::cerr << "Error: Invalid request type." << std::endl;
        return false;
    }

    if (!(iss >> token)) {
        std::cerr << "Error: Missing key." << std::endl;
        return false;
    }
    request.key = std::stoi(token);

    if (request.request_type == 2) {
        if (!(iss >> token)) {
            std::cerr << "Error: Missing value for PUT command." << std::endl;
            return false;
        }
        if (token.size() >= BLOCK_SIZE) {
            std::cerr << "Error: Value exceeds maximum size." << std::endl;
            return false;
        }
        memcpy(request.value, token.c_str(), token.size() + 1); // +1 to include null terminator
    }

    if (iss >> token) {
        std::cerr << "Error: Unexpected token '" << token << "'." << std::endl;
        return false;
    }

    return true;
}

void sendRequest(std::string command, Config& conf, std::vector<RequestInfo>& putRequests, std::vector<RequestInfo>& getRequests) {
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    KVRequest request;

    if (!parseCommand(command, request)) {
        return;
    }

    int num_gateways = conf.gateways.size(); 
    std::string gateway_ip;
    int gateway_cport;

    TimePoint start = Clock::now();

    while (true) {
        int rand = random() % num_gateways;
        auto result = parseAddress(conf.gateways[rand]);
        gateway_ip = std::get<0>(result);
        gateway_cport = std::get<2>(result);

        try {
            rpc::client client(gateway_ip, gateway_cport);
            RequestInfo info;
            switch(request.request_type) {
                case 2:
                case 3:
                    info.reply = client.call("HandleWrite", request).as<std::string>();
                    putRequests.push_back(info);
                    break;
                case 1:
                    info.reply = client.call("HandleRead", request).as<std::string>();
                    getRequests.push_back(info);
                    break;
                default:
                    break;
            }

            TimePoint end = Clock::now();
            info.latency = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            break;
        } catch (const std::exception& e) {
            continue;
        }
    }
}

void writeMetrics(const std::vector<RequestInfo>& requests, const std::string& filename) {
    // Open file in append mode
    std::ofstream file(filename, std::ios_base::app);

    // Calculate average latency and throughput
    int totalRequests = requests.size();
    int totalTime = 0;
    std::vector<int> latencies;
    for (const auto& info : requests) {
        totalTime += info.latency.count();
        latencies.push_back(info.latency.count());
    }

    double averageLatency = totalRequests > 0 ? static_cast<double>(totalTime) / totalRequests : 0.0;
    double throughput = totalRequests > 0 ? static_cast<double>(totalRequests) / totalTime * 1000.0 : 0.0;

    file << "Average Latency: " << averageLatency << " milliseconds" << std::endl;
    file << "Throughput: " << throughput << " requests/second" << std::endl;

    // Calculate P99 latency
    std::sort(latencies.begin(), latencies.end());
    int p99Index = static_cast<int>(requests.size() * 0.99);
    int p99Latency = latencies[p99Index];
    file << "P99 Latency: " << p99Latency << " milliseconds" << std::endl;

    file.close();
}

int main(int argc, char* argv[]) {

    if (argc < 2 ) {
        std::cerr << "Usage: ./client <type>" << std::endl;
        return 1;
    }
    int request_type = atoi(argv[1]);
    // Read config.json file
    Config conf = parseConfig("config.json");

    std::string command;
    std::vector<RequestInfo> putRequests;
    std::vector<RequestInfo> getRequests;

    if (request_type == 0) {
        while (true) {
            std::getline(std::cin, command);
            sendRequest(command, conf, putRequests, getRequests);
        }
    }
    else if (request_type == 1) {
        std::string filePath = argv[2];

        // Open the file
        std::ifstream traceFile(filePath);
        if (!traceFile.is_open()) {
            std::cerr << "Failed to open file: " << filePath << std::endl;
            return 1;
        }

        while (std::getline(traceFile, command)) {
            sendRequest(command, conf, putRequests, getRequests);
        }

        traceFile.close(); 
    }

    writeMetrics(putRequests, "statistics/put_metrics.txt");

    writeMetrics(getRequests, "statistics/get_metrics.txt");

    return 0;
}