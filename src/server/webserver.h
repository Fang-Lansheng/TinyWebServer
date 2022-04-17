//
// Created by Jifan Zhang on 2022/4/11.
//

#ifndef TINYWEBSERVER_SRC_SERVER_WEBSERVER_H
#define TINYWEBSERVER_SRC_SERVER_WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "../log/log.h"
#include "../config/config.h"
#include "../timerheap/timerheap.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnpool.h"
#include "../pool/sqlconnraii.h"
#include "../http/httpconn.h"


class WebServer {
public:

//    WebServer(int port, int trigger_mode, int timeout, bool opt_linger,
//              int database_port, const char* database_user,
//              const char* database_passwd, const char* database_name,
//              int conn_pool_num, int thread_num,
//              bool open_log, int log_level, int log_queue_size);

    WebServer(const Config& config, const DatabaseInfo& db_info);

    ~WebServer();

    void Start();

private:
    bool InitSocket();
    void InitEventMode(int trigger_mode);
    void AddClient(int fd, sockaddr_in addr);

    void DealRead(HttpConn* client);
    void DealWrite(HttpConn* client);
    void DealListen();

    void SendError(int fd, const char* info);
    void ExtentTime(HttpConn* client);
    void CloseConn(HttpConn* client);

    void OnRead(HttpConn* client);
    void OnWrite(HttpConn* client);
    void OnProcess(HttpConn* client);

    static int SetFdNonblock(int fd);

    static const int MAX_FD = 65536;

    int port_{};
    bool open_linger_{};
    int timeout_{};    // milliseconds
    bool is_close_;
    int listen_fd_{};
    char* src_dir_;

    uint32_t listen_event_{};
    uint32_t conn_event_{};

    std::unique_ptr<TimerHeap> timer_;
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;
};


#endif //TINYWEBSERVER_SRC_SERVER_WEBSERVER_H
