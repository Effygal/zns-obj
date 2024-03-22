#pragma once
#include <future>
#include <array>
#include <cstdint>
#include <string>

using key_t = int32_t;
using value_t = int32_t;
using nblk_t = int16_t;
using version_t = int32_t;


// cmd.op types
enum CmdType
{	
	READ = 1,
	APPEND = 2,
};

// gateways translate KV request to cmd
struct cmd {
	CmdType op;
	nblk_t nblks;
	off_t lba;
};

// gateways keep a K-LBAs map in a map manner
// K_LBAs = std::map<key_t, MapVal>, not sure.
// in fact, the MapVal can also serve as a vector clock because each LBAs only grows up.
struct MapVal {
	std::array<off_t, 3> LBAs;
};
// {
// 	std::tuple<off_t, off_t, off_t> LBAs;
// };

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
	DELETE = 3,
};

struct KVRequest 
{
	RequestType request_type;
	key_t key;
	value_t value = 0;
};


struct ReplicationRequest 
{
	key_t key;
	MapVal LBAs;
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
