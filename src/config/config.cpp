//
// Created by Jifan Zhang on 2022/4/15.
//

#include "config.h"

Config::Config() {
    port = 9006;
    trigger_mode = 3;
    timeout = 60000;
    opt_linger = 1;
    conn_pool_num = 12;
    thread_num = 6;
    open_log = 1;
    log_level = 3;
    log_queue_size = 1024;
}

void Config::ParseArg(int argc, char** argv) {
    int opt;
    const char* str = "p:m:o:d:t:l:";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
            case 'p':
                port = std::stoi(optarg);
                break;
            case 'm':
                trigger_mode = std::stoi(optarg);
                break;
            case 'o':
                opt_linger = std::stoi(optarg);
                break;
            case 'd':
                conn_pool_num = std::stoi(optarg);
                break;
            case 't':
                thread_num = std::stoi(optarg);
                break;
            case 'l':
                open_log = std::stoi(optarg);
                break;
            default:
                break;
        }
    }
}

DatabaseInfo::DatabaseInfo(int port, std::string user,
                           std::string passwd, std::string name) {
    db_port = port;
    db_user = std::move(user);
    db_passwd = std::move(passwd);
    db_name = std::move(name);
}

DatabaseInfo::DatabaseInfo(const std::string& path) {
    std::string temp;
    int port;
    std::string user, passwd, database;

    std::ifstream fin;
    fin.open(path);

    if (fin.is_open()) {
        while (!fin.eof()) {
            fin >> temp >> port;
            fin >> temp >> user;
            fin >> temp >> passwd;
            fin >> temp >> database;
        }
        fin.close();

        db_port = port;
        db_user = user;
        db_passwd = passwd;
        db_name = database;
    } else {
        std::cerr << "Error: file " << path << " could not be opened!" << std::endl;
        exit(1);
    }
}
