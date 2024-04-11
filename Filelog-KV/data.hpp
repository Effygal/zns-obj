#pragma once
#include <future>
#include <array>
#include <cstdint>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <rpc/msgpack.hpp>

#define BLOCK_SIZE 4096

using key_t = int32_t;
using nblk_t = int16_t;
// using version_t = int32_t;

/* at gateways
 */

// cmd.op types
// enum CmdType
// {	
// 	READ = 1,
// 	APPEND = 2,
// };
// gateways translate KV request to cmd
struct cmd {
	int8_t op;
	key_t key;
	char value[BLOCK_SIZE] = {0};
	MSGPACK_DEFINE(op, key, value);
};
// gateways keep a K-LBAs map in a map manner
// K_LBAs = std::map<key_t,LBAs>, not sure.
// in fact, the MapVal can also serve as a vector clock because each LBAs only grows up.
struct LBAs {
	std::array<off_t, 3> lbas;
	MSGPACK_DEFINE(lbas); 
};

struct AppendReply {
	int16_t _logger_id;
    key_t key;
    off_t lba;
	MSGPACK_DEFINE_ARRAY(_logger_id, key, lba);
};

struct ReadReply {
	key_t key;
	std::string value;
	MSGPACK_DEFINE(key, value);
};

struct KVRequest 
{
	int8_t request_type;
	key_t key;
	char value[BLOCK_SIZE] = {0};
	MSGPACK_DEFINE(request_type, key, value);
};

struct ReplicationRequest 
{
	key_t key;
	LBAs lbas;
};

struct ReplicationResp 
{
	bool success;
};

struct CatchupRequest
{
	
};

struct CatchupResp
{
	
};

enum ConnType
{
	CLIENT_REQUEST,
	MASTER_REPLICATION,
	REPLICATION_CATCHUP,
	INVALID,
};

struct ConnHeader 
{
	ConnType type;
};


/*
	at loggers
 */
struct header_t {
	key_t key;
	ssize_t length;
	// char padding[BLOCK_SIZE-sizeof(key_t)-sizeof(ssize_t)] = {0};
};

struct LogEnt {
	header_t metadata;
    char data[BLOCK_SIZE-sizeof(header_t)] = {0};

	LogEnt (key_t key, std::string value){
		metadata.key = key;
		metadata.length = value.size();
		std::strcpy(data, value.c_str());
	}
	key_t get_key() {
		return metadata.key;
	}
	ssize_t get_length() {
		return metadata.length;
	}
	char* get_value() {
		return data;
	}
};
