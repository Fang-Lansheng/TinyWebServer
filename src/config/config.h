//
// Created by Jifan Zhang on 2022/4/15.
//

#ifndef TINYWEBSERVER_SRC_CONFIG_CONFIG_H
#define TINYWEBSERVER_SRC_CONFIG_CONFIG_H

#include <string>
#include <fstream>
#include <iostream>
#include <getopt.h>
#include <unistd.h>

class DatabaseInfo {
public:
    DatabaseInfo(int port, std::string user, std::string passwd, std::string db_name);
    explicit DatabaseInfo(const std::string& path);

    int db_port = 3006;
    std::string db_user = "root";
    std::string db_passwd = "passwd";
    std::string db_name = "database";
};

class Config {
public:
    Config();

    void ParseArg(int argc, char* argv[]);

    // Server port. Default: 9006.
    int port;

    // Trigger mode. Default: ET mode.
    int trigger_mode;

    // Timeout (in milliseconds). Default: 60000.
    int timeout;

    // Weather to Opt linger. Default: 1 (true).
    int opt_linger;

    // Num of connection in database connection pool. Default: 12.
    size_t conn_pool_num;

    // Num of thread in thread pool. Default: 4
    // (my Linux server has a CPU with 2 cores and 2 threads per core).
    int thread_num;

    // Weather to open log. Default: 1 (true).
    int open_log;

    // Log level. Default: 1 (INFO).
    int log_level;

    // Log queue size. Default: 1024;
    int log_queue_size;
};


#endif //TINYWEBSERVER_SRC_CONFIG_CONFIG_H
