//
// Created by Jifan Zhang on 2022/4/13.
//

#ifndef TINYWEBSERVER_SRC_POOL_SQLCONNPOOL_H
#define TINYWEBSERVER_SRC_POOL_SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>

#include "../log/log.h"

class SqlConnPool {
public:

    static SqlConnPool* Instance();

    MYSQL* GetConn();

    void FreeConn(MYSQL* conn);

    size_t GetFreeConnCount();

    // Database connections pool initialization.
    // Add a MYSQL* connection handler to the connection queue.
    void Init(const char* host, int port,
              const char* user, const char* passwd,
              const char* database_name,
              size_t conn_size);

    void ClosePool();

private:
    // ctor
    SqlConnPool();

    // dtor
    ~SqlConnPool();

private:
    // Max number of MySQL connections.
    size_t MAX_CONN_{};

    // Number of connections.
    int use_count_;

    //
    int free_count_;

    std::queue<MYSQL*> conn_queue_;

    std::mutex mutex_;

    sem_t sem_id_{};

};


#endif //TINYWEBSERVER_SRC_POOL_SQLCONNPOOL_H
