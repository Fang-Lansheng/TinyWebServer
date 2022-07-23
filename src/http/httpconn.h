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
    // ctor
    HttpConn();

    // dtor
    ~HttpConn();

    // HTTP connection initialization
    void Init(int sock_fd, const sockaddr_in& addr);

    // Process the read request from the client and
    // return the length of the file.
    ssize_t Read(int* save_errno);

    // Process the write request from the client and
    // return the length of the file.
    ssize_t Write(int* save_errno);

    // Close the HTTP connection.
    void Close();

    // Get the socket fd.
    int GetFd() const;

    // Get the port number.
    int GetPort() const;

    // Get the Internet address.
    const char* GetIP() const;

    // Get the socket address.
    sockaddr_in GetAddr() const;

    // Parse the HTTP request or generate the HTTP response
    bool Process();

    // The length of data to be written
    size_t ToWriteBytes();

    // HTTP keep-alive
    bool IsKeepAlive() const;

public:
    // Is edge trigger.
    static bool is_et;

    // Current working dir.
    static const char* src_dir;

    // Number of HTTP connections.
    static std::atomic<int> user_count;

private:
    // Socket file descriptor.
    int fd_;

    // Internet socket address.
    struct sockaddr_in addr_{};

    // Connection status.
    bool is_close_;

    // Structure for scatter/gather I/O.
    struct iovec iov_[2]{};

    // Number of `iovec`.
    int iov_count_{};

    // Read buffer.
    Buffer read_buffer_;

    // Write buffer.
    Buffer write_buffer_;

    // Requester to parse the HTTP request message.
    HttpRequest request_;

    // Responser to generate the HTTP response message.
    HttpResponse response_;
};


#endif //TINYWEBSERVER_SRC_HTTP_HTTPCONN_H
