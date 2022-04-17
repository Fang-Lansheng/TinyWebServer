//
// Created by Jifan Zhang on 2022/4/13.
//

#include "sqlconnpool.h"


SqlConnPool::SqlConnPool() {
    use_count_ = 0;
    free_count_ = 0;
}

SqlConnPool::~SqlConnPool() {
    ClosePool();
}

SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool conn_pool;
    return &conn_pool;
}

MYSQL* SqlConnPool::GetConn() {
    MYSQL* sql;
    if (conn_queue_.empty()) {
        LOG_WARN("SQL connection pool is busy!");
        return nullptr;
    }

    sem_wait(&sem_id_);

    {
        std::lock_guard<std::mutex> locker(mutex_);
        sql = conn_queue_.front();
        conn_queue_.pop();
    }

    return sql;
}

void SqlConnPool::FreeConn(MYSQL* conn) {
    assert(conn);
    std::lock_guard<std::mutex> locker(mutex_);
    conn_queue_.push(conn);
    sem_post(&sem_id_);
}

size_t SqlConnPool::GetFreeConnCount() {
    std::lock_guard<std::mutex> locker(mutex_);
    return conn_queue_.size();
}

void SqlConnPool::Init(const char* host, int port,
                       const char* user, const char* passwd,
                       const char* database_name,
                       size_t conn_size) {
    assert(conn_size > 0);
    for (size_t i = 0; i < conn_size; ++i) {
        MYSQL* sql = mysql_init(nullptr);
        if (!sql) {
            LOG_ERROR("MySQL initialization error! Maybe there was "
                      "insufficient memory to allocate a new object");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host, user, passwd, database_name,
                                 port, nullptr, 0);
        if (!sql) {
            LOG_ERROR("MySQL connection error! Check whether the user "
                      "name and password are correct")
        }
        conn_queue_.push(sql);
    }
    MAX_CONN_ = conn_size;
    sem_init(&sem_id_, 0, MAX_CONN_);
}

void SqlConnPool::ClosePool() {
    std::lock_guard<std::mutex> locker(mutex_);
    while (!conn_queue_.empty()) {
        auto conn = conn_queue_.front();
        conn_queue_.pop();
        mysql_close(conn);
    }
    mysql_library_end();
}

