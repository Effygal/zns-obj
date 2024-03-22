#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <tuple>
#include <vector>
#include <map>

#define BLOCK_SIZE 4096
#define APPEND 1
#define READ 0

struct cmd {
    int op; // append; read
    int n_blks;
    int blk_num;
};

struct reply {
    int key_version;
    off_t lba;
};

struct logent {
    int key_ver;
    int key;
    char data[BLOCK_SIZE];
};

std::map<int, std::tuple<int, int, int>> key_lbas_map;
