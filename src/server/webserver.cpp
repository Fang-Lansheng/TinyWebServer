//
// Created by Jifan Zhang on 2022/4/11.
//

#include "webserver.h"

//WebServer::WebServer(int port, int trigger_mode, int timeout, bool opt_linger,
//                     int database_port, const char* database_user,
//                     const char* database_passwd, const char* database_name,
//                     int conn_pool_num, int thread_num,
//                     bool open_log, int log_level, int log_queue_size) {
//    src_dir_ = getcwd(nullptr, 256);
//    assert(src_dir_);
//    strncat(src_dir_, "/assets", 16);
//
//    HttpConn::user_count = 0;
//    HttpConn::src_dir = src_dir_;
//    SqlConnPool::Instance()->Init("localhost", database_port,
//                                  database_user, database_passwd,
//                                  database_name, conn_pool_num);
//
//    InitEventMode(trigger_mode);
//    if (!InitSocket()) {
//        is_close_ = true;
//    }
//
//    if (open_log) {
//        Log::Instance()->Init(log_level, "./logs", ".log", log_queue_size);
//        if (is_close_) {
//            LOG_ERROR("========== Server init error! ==========");
//        } else {
//            LOG_INFO("========== Server init ==========");
//            LOG_INFO("Port: %d, OpenLinger: %s",
//                     port, opt_linger ? "true" : "false");
//            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
//                     (listen_event_ & EPOLLET ? "ET" : "LT"),
//                     (conn_event_ & EPOLLET ? "ET" : "LT"));
//            LOG_INFO("LogSys level: %d", log_level);
//            LOG_INFO("source dirname: %s", HttpConn::src_dir);
//            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d",
//                     conn_pool_num, thread_num);
//        }
//    }
//}


WebServer::WebServer(const Config& config, const DatabaseInfo& db_info):
    port_(config.port), open_linger_(config.opt_linger), timeout_(config.timeout),
    is_close_(false), timer_(new TimerHeap()),
    thread_pool_(new ThreadPool(config.thread_num)), epoller_(new Epoller()) {

    src_dir_ = getcwd(nullptr, 256);
    assert(src_dir_);
    strncat(src_dir_, "/assets", 16);

    HttpConn::user_count = 0;
    HttpConn::src_dir = src_dir_;
    SqlConnPool::Instance()->Init("localhost", db_info.db_port,
                                  db_info.db_user.c_str(),
                                  db_info.db_passwd.c_str(),
                                  db_info.db_name.c_str(),
                                  config.conn_pool_num);

    InitEventMode(config.trigger_mode);
    if (!InitSocket()) {
        is_close_ = true;
    }

    if (config.open_log) {
        Log::Instance()->Init(config.log_level, "./logs", ".log",
                              config.log_queue_size);

        if (is_close_) {
            LOG_ERROR("========== Server init error! ==========");
        } else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port: %d, OpenLinger: %s",
                     config.port, config.opt_linger ? "true" : "false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                     (listen_event_ & EPOLLET ? "ET" : "LT"),
                     (conn_event_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: %d", config.log_level);
            LOG_INFO("source dirname: %s", HttpConn::src_dir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d",
                     config.conn_pool_num, config.thread_num);
        }
    }
}


WebServer::~WebServer() {
    close(listen_fd_);
    is_close_ = true;
    free(src_dir_);
    SqlConnPool::Instance()->ClosePool();
}

void WebServer::Start() {
    int milliseconds = -1;   // epoll wait timeout = -1
    if (!is_close_) {
        LOG_INFO("")
        LOG_INFO("========== Server start ==========");
    }
    while (!is_close_) {
        if (timeout_ > 0) {
            milliseconds = timer_->GetNextTick();
        }
        int event_count = epoller_->Wait(milliseconds);
        for (int i = 0; i < event_count; ++i) {
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if (fd == listen_fd_) {
                DealListen();
            } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                CloseConn(&users_[fd]);
            } else if (events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                DealRead(&users_[fd]);
            } else if (events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                DealWrite(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected event!");
            }
        }
    }
}

bool WebServer::InitSocket() {
    struct sockaddr_in addr{};
    if (port_ > 65535 || port_ < 1024) {
        LOG_ERROR("Port %d is out of bound!", port_);
        return false;
    }
    addr.sin_family = AF_INET;                  // IP protocol family
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   // Address to accept any incoming messages
    addr.sin_port = htons(port_);

    struct linger opt_linger = {0};
    if (open_linger_) {
        opt_linger.l_onoff = 1;                 // Nonzero to linger on close
        opt_linger.l_linger = 1;
    }

    // Create a new socket of type `SOCK_STREAM` in domain `AF_INET`
    // Return a file descriptor for the new socket, or -1 for errors.
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        LOG_ERROR("Socket creation failed at port %d", port_);
        return false;
    }

    int socket_ret;
    // Set socket FD`s option `SO_LINGER` at protocol level `SOL_SOCKET`
    // to `opt_linger`.
    // Returns 0 on success, -1 for errors.
    socket_ret = setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER,
                            &opt_linger, sizeof(opt_linger));
    if (socket_ret < 0) {
        close(listen_fd_);
        LOG_ERROR("Init linger failed!");
        return false;
    }

    // Reuse port
    // Only the last socket will receive data normally (?)
    int opt_val = -1;
    socket_ret = setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR,
                            static_cast<const void*>(&opt_val),
                            sizeof(opt_val));
    if (socket_ret < 0) {
        close(listen_fd_);
        LOG_ERROR("Socket setting `setsockopt` error!");
        return false;
    }

    // Give the socket FD the local address ADDR
    socket_ret = bind(listen_fd_, reinterpret_cast<const sockaddr*>(&addr),
                      sizeof(addr));
    if (socket_ret < 0) {
        close(listen_fd_);
        LOG_ERROR("Bind port: %d error!", port_);
        return false;
    }

    // Prepare to accept connections on socket FD. N connection
    // requests will be queued before further requests are refused.
    // Returns 0 on success, -1 for errors.
    socket_ret = listen(listen_fd_, 6);
    if (socket_ret < 0) {
        close(listen_fd_);
        LOG_ERROR("Listen port: %d error!", port_);
        return false;
    }

    socket_ret = epoller_->AddFd(listen_fd_, listen_event_ | EPOLLIN);
    if (socket_ret == 0) {
        close(listen_fd_);
        LOG_ERROR("Add listen error!");
        return false;
    }

    SetFdNonblock(listen_fd_);
    LOG_INFO("Server port: %d", port_);
    return true;
}

void WebServer::InitEventMode(int trigger_mode) {
    listen_event_ = EPOLLRDHUP;
    conn_event_ = EPOLLONESHOT | EPOLLRDHUP;

    switch (trigger_mode) {
        case 0:
            break;
        case 1:
            conn_event_ |= EPOLLET;
            break;
        case 2:
            listen_event_ |= EPOLLET;
            break;
        default:    // trigger_mode == 3
            conn_event_ |= EPOLLET;
            listen_event_ |= EPOLLET;
            break;
    }

    HttpConn::is_et = (conn_event_ & EPOLLET);
}

void WebServer::AddClient(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].Init(fd, addr);
    if (timeout_ > 0) {
//        timer_->Add(fd, timeout_,
//                    [this, capture0 = &users_[fd]] { CloseConn(capture0); });
        timer_->Add(fd, timeout_,
                    std::bind(&WebServer::CloseConn, this, &users_[fd]));
    }

    epoller_->AddFd(fd, EPOLLIN | conn_event_);
    SetFdNonblock(fd);
    LOG_INFO("Client [%d] in.", users_[fd].GetFd());
}

void WebServer::DealRead(HttpConn* client) {
    assert(client);
    ExtentTime(client);
//    thread_pool_->AddTask([this, client] { OnRead(client); });
    thread_pool_->AddTask(std::bind(&WebServer::OnRead, this, client));
}

void WebServer::DealWrite(HttpConn* client) {
    assert(client);
    ExtentTime(client);
//    thread_pool_->AddTask([this, client] { OnWrite(client); });
    thread_pool_->AddTask(std::bind(&WebServer::OnWrite, this, client));
}

void WebServer::DealListen() {
    struct sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listen_fd_, (struct sockaddr*)&addr, &len);
        if (fd <= 0) {
            return;
        } else if (HttpConn::user_count >= MAX_FD) {
            SendError(fd, "Server is busy!");
            LOG_WARN("Clients are full!");
            return;
        } else {
            AddClient(fd, addr);
        }
    } while (listen_event_ & EPOLLET);
}

void WebServer::SendError(int fd, const char* info) {
    assert(fd > 0);
    auto socket_ret = send(fd, info, strlen(info), 0);
    if (socket_ret < 0) {
        LOG_WARN("Send error to client [%d] failed!", fd);
    }
    close(fd);
}

void WebServer::ExtentTime(HttpConn* client) {
    assert(client);
    if (timeout_ > 0) {
        timer_->Adjust(client->GetFd(), timeout_);
    }
}

void WebServer::CloseConn(HttpConn* client) {
    assert(client);
    LOG_INFO("Client [%d] quit.", client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->Close();
}

void WebServer::OnRead(HttpConn* client) {
    assert(client);
    int read_errno = 0;
    auto ret = client->Read(&read_errno);
    if (ret <= 0 && read_errno != EAGAIN) {
        CloseConn(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnWrite(HttpConn* client) {
    assert(client);
    int write_errno = 0;
    auto ret = client->Write(&write_errno);
    if (client->ToWriteBytes() == 0) {
        // Transportation ended
        if (client->IsKeepAlive()) {
            OnProcess(client);
            return;
        }
    } else if (ret < 0) {
        if (write_errno == EAGAIN) {
            // continue transporting
            epoller_->ModFd(client->GetFd(), conn_event_ | EPOLLOUT);
            return;
        }
    }
    CloseConn(client);
}

void WebServer::OnProcess(HttpConn* client) {
    if (client->Process()) {
        epoller_->ModFd(client->GetFd(), conn_event_ | EPOLLOUT);
    } else {
        epoller_->ModFd(client->GetFd(), conn_event_ | EPOLLIN);
    }
}

int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}



