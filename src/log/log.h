//
// Created by Jifan Zhang on 2022/4/11.
//

#ifndef TINYWEBSERVER_SRC_LOG_LOG_H
#define TINYWEBSERVER_SRC_LOG_LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <ctime>
#include <cstring>
#include <cstdarg>
#include <cassert>
#include <sys/stat.h>

#include "blockdeque.h"
#include "../buffer/buffer.h"


class Log {
public:
    void Init(int level = 1,
              const char* path = "./log",
              const char* suffix = ".log",
              int max_queue_capacity = 1024);

    static Log* Instance();

    static void FlushLogThread();

    void Write(int level, const char* format, ...);

    void Flush();

    int GetLevel();

    void SetLevel(int level);

    bool IsOpen() const;

private:
    Log();

    void AppendLogLevelTitle(int level);

    virtual ~Log();

    void AsyncWrite();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path_{};
    const char* suffix_{};

    int max_lines_{};
    int line_count_;
    int to_day_;

    bool is_open_{};

    Buffer buffer_;
    int level_{};
    bool is_async_;

    FILE* fp_;
    std::unique_ptr<BlockDeque<std::string>> deque_;
    std::unique_ptr<std::thread> write_thread_;
    std::mutex mutex_;
};

#define LOG_BASE(level, format, ...)                        \
    do {                                                    \
        Log* log = Log::Instance();                         \
        if (log->IsOpen() && log->GetLevel() >= level) {    \
            log->Write(level, format, ##__VA_ARGS__);       \
            log->Flush();                                   \
        }                                                   \
    } while (0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...)  do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...)  do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //TINYWEBSERVER_SRC_LOG_LOG_H
