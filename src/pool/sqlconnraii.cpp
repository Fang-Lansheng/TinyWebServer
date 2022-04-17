//
// Created by Jifan Zhang on 2022/4/13.
//

#include "sqlconnraii.h"

SqlConnRaii::SqlConnRaii(MYSQL** sql, SqlConnPool* conn_pool) {
    assert(conn_pool);
    *sql = conn_pool->GetConn();
    sql_ = *sql;
    conn_pool_ = conn_pool;
}

SqlConnRaii::~SqlConnRaii() {
    if (sql_) {
        conn_pool_->FreeConn(sql_);
    }
}
