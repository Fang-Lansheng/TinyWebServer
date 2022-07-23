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

    /*
    // ctor
    WebServer(int port, int trigger_mode, int timeout, bool opt_linger,
              int database_port, const char* database_user,
              const char* database_passwd, const char* database_name,
              int conn_pool_num, int thread_num,
              bool open_log, int log_level, int log_queue_size);
    */

    // ctor
    WebServer(const Config& config, const DatabaseInfo& db_info);

    // dtor
    ~WebServer();

    // Start the web server. Until the server is shut down, events
    // are continuously retrieved and processed by the server.
    void Start();

private:
    // Socket connection initialization.
    // Returns `true` in case of success, `false` in case of error.
    bool InitSocket();

    // Event mode initialization
    // (controlled by a single parameter `trigger_mode`)
    //
    // `trigger_mode` | listen mode | connection mode
    // -------------- | ----------- | ---------------
    // 0              | LT          | LT
    // 1              | LT          | ET
    // 2              | ET          | LT
    // 3              | ET          | ET
    void InitEventMode(int trigger_mode);

    //
    void AddClient(int fd, sockaddr_in addr);

    // Deal with the read request from the client.
    // The reader thread is added to the thread pool.
    void DealRead(HttpConn* client);

    // Deal with the write request from the client.
    // The writer thread is added to the thread pool.
    void DealWrite(HttpConn* client);

    // Build up a connection with a new client.
    void DealListen();

    //
    void SendError(int fd, const char* info);

    //
    void ExtentTime(HttpConn* client);

    // Close the connection with one client
    void CloseConn(HttpConn* client);

    // Scatter read.
    void OnRead(HttpConn* client);

    // Gather write.
    void OnWrite(HttpConn* client);

    // Reset the fd of the client for subsequent processing.
    void OnProcess(HttpConn* client);

    //
    static int SetFdNonblock(int fd);

private:
    static const int MAX_FD = 65536;

    int port_{};            // port
    bool open_linger_{};
    int timeout_{};         // milliseconds
    bool is_close_;
    int listen_fd_{};
    char* src_dir_;

    uint32_t listen_event_{};
    uint32_t conn_event_{};

    std::unique_ptr<TimerHeap> timer_;
    std::unique_ptr<ThreadPool> thread_pool_;   // thread pool
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;
};


#endif //TINYWEBSERVER_SRC_SERVER_WEBSERVER_H
