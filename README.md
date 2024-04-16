
# kv-filelog ![MIT](https://img.shields.io/badge/license-MIT-blue.svg) 
## Status

**[kv-filelog is a toy testbed for the zns-obj project](https://https://github.com/Effygal/zns-obj)**

## Overview

`kv-filelog` FilelogKV is a two-layer (gateways and loggers) distributed KV storage system; built on top of the conventional file system, with the underlying file on each logger serving as the log for KV storage; data is stored in an append manner; we utilize the current LBA of the file as a weak version ID; providing both client, gateway server, and logger server implementations. It is built using modern C++17, and as such, requires a recent compiler.



## Look&feel

### Gateway Server

```cpp
/*
    ------------------------------------
    Gateways
    ------------------------------------
    Usage: ./gateway <gateway_id>
*/
...
std::thread server_thread([&gateway]() {
    rpc::server srv(gateway.cport); 
    srv.bind("HandleRead", [&gateway](KVRequest command) -> std::string{
        std::string reply = gateway.HandleRead(command);
        return reply;
    });

    srv.bind("HandleWrite", [&gateway](KVRequest command) -> std::string{
        std::string reply = gateway.HandleWrite(command);
        return reply;
    });
    srv.run();
});

std::thread catchup_thread([&gateway]() {
    rpc::server bsrv(gateway.bport);
    bsrv.bind("HandleCatchup", [&gateway](key_t key, LBAs lbas) {
        gateway.HandleCatchup(key, lbas);
    });
    bsrv.run();
});

std::thread recovery_thread([&gateway]() {
    gateway.HandleRecovery();
});
...

```
The gateway servers act as intermediaries between the clients issuing KV requests and the loggers responsible for logging the KV store. When a client-selected gateway receives a KV request, it translates the request into Append commands and forwards them to all loggers. In the case of a write request, including overwrites, the gateway receives replies from all loggers containing the corresponding logical block addresses (LBAs) for the requested key, which only increase over time. It then broadcasts these LBAs to all other gateways, detecting failures in the process. For read requests, the gateway selects a random logger, sends the LBA for the requested key to the chosen logger, and retrieves the corresponding value.The recovery thread aggressively runs on an independent thread, constantly waiting for the failed peers list to become non-empty on a conditional variable. Once awakened, the awaked gateway responsible for recovering of the entire list of failed peers.

### Logger Server

```cpp
/*
    ------------------------------------
    Loggers
    ------------------------------------
    Usage: ./logger <logger_id>
*/

off_t Logger::Append(const LogEnt& logent) {
    ...
    ssize_t bytes_written = pwrite(_fd, &logent, sizeof(LogEnt), _cur_lba);
    ...
    off_t offset = lseek(_fd, bytes_written, SEEK_CUR); 
    ...
    _cur_lba = offset; /* emulate current write pointer of ZNSSD */
    ...
    return offset;
}

ReadReply Logger::ReadThread(cmd& request, off_t lba) {
    ...
    ssize_t bytes_read = pread(_fd, buffer, sizeof(buffer), lba - BLOCK_SIZE);
    ... 
    ReadReply reply = {request.key, value};
    return reply;
}

```

All loggers within the cluster talk to all gateways for append request; the chosen logger talks to the gateway who made the choice for read request; the underlying implementation of append is in terms of pwrite and lseek; the underlying implementation of read is in pread; two services running on two threads.
Loggers are assumed to never fail (although we implemented failure handles for loggers as well), because they are pretending to be ZNSSDs that are persistent.

## Client

```cpp
#include "rpc/client.h"
    ...
    rpc::client client(random_gateway_ip, random_gateway_cport);
    switch(request.request_type) {
        case PUT:
        case DEL:
            reply = client.call("HandleWrite", request).as<std::string>();
            break;
        case GET:
            reply = client.call("HandleRead", request).as<std::string>();
            break;
        ...
    }
```

# Status
* Currently accepts PUT, GET and DEL requests;
* Failure tolerant as long as at least one gateway and one logger alive;
* Scalable for both the gateways, loggers and the clients;
* Failure detection only be triggered when received PUT request;
* In scenarios where only one gateway server is alive, and fails immediately after previously failed peers come back online, recovery cannot be assured. Consequently, the gateway server returning online may not undergo complete recovery, resulting in data loss;
* Since the failed peers list isn't replicated, in scenarios where the one gateway server who acquired lock to handle the recovery (considered as recovery manager) fails before completing the recovery for the entire list of peers, the recovery process would be incomplete, potentially leading to data loss if everyone fails in the future except the gateway that was not fully recovered;
* Use an aggressive recovery implementation without implementing checkpoints, resulting in linear asymptotic cost.

# Dependencies

`kv-filelog` builds on the efforts of below projects and libs. In no particular order:
  * [rpclib implementation for C++](https://github.com/rpclib/rpclib)([website](http://rpclib.net/))
   * [MessagePack implementation for C and C++](https://github.com/msgpack/msgpack-c) by Takatoshi Kondo ([website](http://msgpack.org/))
