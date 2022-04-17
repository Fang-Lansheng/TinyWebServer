//
// Created by Jifan Zhang on 2022/4/11.
//

#ifndef TINYWEBSERVER_SRC_BUFFER_BUFFER_H
#define TINYWEBSERVER_SRC_BUFFER_BUFFER_H

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <atomic>
#include <cassert>


class Buffer {
public:
    explicit Buffer(int init_buffer_size = 1024);
    ~Buffer();

    size_t WritableBytes() const;
    size_t ReadableBytes() const;
    size_t PrependableBytes() const;
    void EnsureWritable(size_t len);
    void HasWritten(size_t len);
    const char* Peek() const;

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);
    void RetrieveAll();
    std::string RetrieveAllToStr();

    char* BeginWrite();
    const char* BeginWriteConst() const;

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buffer);

    ssize_t ReadFd(int fd, int* ptr_errno);
    ssize_t WriteFd(int fd, int* ptr_errno);

private:
    char* BeginPtr();
    const char* BeginPtr() const;
    void MakeSpace(size_t len);

    std::vector<char> buffer_;
    std::atomic<std::size_t> read_pos_;
    std::atomic<std::size_t> write_pos_;
};


#endif //TINYWEBSERVER_SRC_BUFFER_BUFFER_H
