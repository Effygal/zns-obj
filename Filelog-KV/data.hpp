#pragma once
#include <future>
#include <array>
#include <cstdint>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include "tcp_utils.hpp"
#include "udp_utils.hpp"
#include <iostream>

#define BLOCK_SIZE 4096

using key_t = int32_t;
using nblk_t = int16_t;
// using version_t = int32_t;

/* at gateways
 */

// cmd.op types
enum CmdType
{	
	READ = 1,
	APPEND = 2,
};
// gateways translate KV request to cmd
struct cmd {
	CmdType op;
	key_t key;
	char value[BLOCK_SIZE] = {0};	
};
// gateways keep a K-LBAs map in a map manner
// K_LBAs = std::map<key_t,LBAs>, not sure.
// in fact, the MapVal can also serve as a vector clock because each LBAs only grows up.
struct LBAs {
	std::array<off_t, 3> lbas;
};

struct LoggerReply {
	int16_t _logger_id;
    key_t key;
    off_t lba;
};

/*
class LoggedMap
{
private:
	std::vector<MapOp> _oplog;
	std::shared_ptr<SyncMap<customerid_t, record_t>> _map;

public:
	LoggedMap();
	~LoggedMap();

	// Shouldn't be copied
	LoggedMap(LoggedMap &src) = delete;
	LoggedMap &operator=(LoggedMap &src) = delete;

	LoggedMap(LoggedMap &&src);
	LoggedMap &operator=(LoggedMap &&src);

	oplogidx_t add_op(MapOp op);
	void apply_up_to(oplogidx_t idx);
};
*/
enum RequestType
{
	GET = 1,
	PUT = 2,
	DEL = 3,
};

struct KVRequest 
{
	RequestType request_type;
	key_t key;
	char value[BLOCK_SIZE] = {0};
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

	void set_key(key_t key) {
		metadata.key = key;
	}
	void set_length(ssize_t length) {
		metadata.length = length;
	}
	key_t get_key() {
		return metadata.key;
	}
	ssize_t get_length() {
		return metadata.length;
	}
	void set_value(std::string value) {
		strcpy(data, value.c_str());
	}
	char* get_value() {
		return data;
	}
};
