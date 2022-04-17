//
// Created by Jifan Zhang on 2022/4/13.
//

#ifndef TINYWEBSERVER_SRC_HTTP_HTTPCONN_H
#define TINYWEBSERVER_SRC_HTTP_HTTPCONN_H

#include <sys/types.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cerrno>

#include "../log/log.h"
#include "../pool/sqlconnraii.h"
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn {
public:
    HttpConn();

    ~HttpConn();

    void Init(int sock_fd, const sockaddr_in& addr);

    ssize_t Read(int* save_errno);

    ssize_t Write(int* save_errno);

    void Close();

    int GetFd() const;

    int GetPort() const;

    const char* GetIP() const;

    sockaddr_in GetAddr() const;

    bool Process();

    size_t ToWriteBytes();

    bool IsKeepAlive() const;

    static bool is_et;
    static const char* src_dir;
    static std::atomic<int> user_count;

private:
    int fd_;
    struct sockaddr_in addr_{};
    bool is_close_;
    int iov_count_{};
    struct iovec iov_[2]{};

    Buffer read_buffer_;
    Buffer write_buffer_;

    HttpRequest request_;
    HttpResponse response_;
};


#endif //TINYWEBSERVER_SRC_HTTP_HTTPCONN_H
