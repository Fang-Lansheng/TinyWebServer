//
// Created by Jifan Zhang on 2022/4/13.
//

#ifndef TINYWEBSERVER_SRC_POOL_SQLCONNRAII_H
#define TINYWEBSERVER_SRC_POOL_SQLCONNRAII_H

#include "sqlconnpool.h"


class SqlConnRaii {
public:
    SqlConnRaii(MYSQL** sql, SqlConnPool* conn_pool);

    ~SqlConnRaii();

private:
    MYSQL* sql_;
    SqlConnPool* conn_pool_;
};


#endif //TINYWEBSERVER_SRC_POOL_SQLCONNRAII_H
