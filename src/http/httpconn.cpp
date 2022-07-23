//
// Created by Jifan Zhang on 2022/4/13.
//

#include "httpconn.h"

bool HttpConn::is_et;
const char* HttpConn::src_dir;
std::atomic<int> HttpConn::user_count;


HttpConn::HttpConn() {
    fd_ = -1;
    addr_ = { 0 };
    is_close_ = true;
}

HttpConn::~HttpConn() {
    Close();
}

void HttpConn::Init(int sock_fd, const sockaddr_in &addr) {
    assert(sock_fd > 0);
    ++user_count;
    addr_ = addr;
    fd_ = sock_fd;
    write_buffer_.RetrieveAll();
    read_buffer_.RetrieveAll();
    is_close_ = false;
    LOG_INFO("Client [%d](%s:%d) in, user count: %d",
             fd_, GetIP(), GetPort(), static_cast<int>(user_count));
}

ssize_t HttpConn::Read(int* save_errno) {
    ssize_t len;
    do {
        len = read_buffer_.ReadFd(fd_, save_errno);
        if (len <= 0) {
            break;
        }
    } while (is_et);

    return len;
}

ssize_t HttpConn::Write(int* save_errno) {
    ssize_t len;
    do {
        len = writev(fd_, iov_, iov_count_);
        if (len <= 0) {
            *save_errno = errno;
            break;
        }

        if (iov_[0].iov_len + iov_[1].iov_len == 0) {
            // Transportation ended
            break;
        } else if (static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base +
                    (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len) {
                write_buffer_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        } else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            write_buffer_.Retrieve(len);
        }
    } while (is_et || ToWriteBytes() > 10240);

    return len;
}

void HttpConn::Close() {
    response_.UnmapFile();
    if (!is_close_) {
        is_close_ = true;
        --user_count;
        close(fd_);
        LOG_INFO("Client [%d](%s:%d) quit, user count: %d",
                 fd_, GetIP(), GetPort(), static_cast<int>(user_count));
    }
}

int HttpConn::GetFd() const {
    return fd_;
}

int HttpConn::GetPort() const {
    return addr_.sin_port;
}

const char* HttpConn::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}

sockaddr_in HttpConn::GetAddr() const {
    return addr_;
}

bool HttpConn::Process() {
    // HTTP requester initialization
    request_.Init();
    if (read_buffer_.ReadableBytes() <= 0) {
        return false;
    } else if (request_.Parse(read_buffer_)) {
        // Parse the HTTP request and initialize the responser
        LOG_DEBUG("%s", request_.Path().c_str());
        response_.Init(src_dir, request_.Path(), request_.IsKeepAlive(), 200);
    } else {
        response_.Init(src_dir, request_.Path(), false, 400);
    }

    // Generate the HTTP response and put it into the write buffer
    response_.MakeResponse(write_buffer_);

    // Response head
    // `iov_base` is the pointer to data.
    iov_[0].iov_base = const_cast<char*>(write_buffer_.Peek());
    // `iov_len` is the length of data.
    iov_[0].iov_len = write_buffer_.ReadableBytes();
    iov_count_ = 1;

    // Files
    if (response_.FileLength() > 0 && response_.File()) {
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLength();
        iov_count_ = 2;
    }
    LOG_DEBUG("filesize: %d, %d to %d",
              response_.FileLength(), iov_count_, ToWriteBytes());
    return true;
}

size_t HttpConn::ToWriteBytes() {
    return iov_[0].iov_len + iov_[1].iov_len;
}

bool HttpConn::IsKeepAlive() const {
    return request_.IsKeepAlive();
}


