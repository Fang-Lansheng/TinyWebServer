//
// Created by Jifan Zhang on 2022/4/11.
//

#include "log.h"

Log::Log() {
    line_count_ = 0;
    is_async_ = false;
    write_thread_ = nullptr;
    deque_ = nullptr;
    to_day_ = 0;
    fp_ = nullptr;
}

Log::~Log() {
    if (write_thread_ && write_thread_->joinable()) {
        while (!deque_->Empty()) {
            deque_->Flush();
        }
        deque_->Close();
        write_thread_->join();
    }
    if (fp_) {
        std::lock_guard<std::mutex> locker(mutex_);
        Flush();
        fclose(fp_);
    }
}

void Log::Init(int level, const char* path, const char* suffix, int max_queue_capacity) {
    is_open_ = true;
    level_ = level;

    if (max_queue_capacity > 0) {
        is_async_ = true;
        if (!deque_) {
            std::unique_ptr<BlockDeque<std::string>> new_deque(
                    new BlockDeque<std::string>);
            deque_ = std::move(new_deque);
            std::unique_ptr<std::thread> new_thread(
                    new std::thread(FlushLogThread));
            write_thread_ = std::move(new_thread);
        }
    } else {
        is_async_ = false;
    }

    line_count_ = 0;

    time_t timer = time(nullptr);
    struct tm* sys_time = localtime(&timer);
    struct tm t = *sys_time;
    path_ = path;
    suffix_ = suffix;
    char file_name[LOG_NAME_LEN] = {0};
    snprintf(file_name, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s",
             path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    to_day_ = t.tm_mday;

    {
        std::lock_guard<std::mutex> locker(mutex_);
        buffer_.RetrieveAll();
        if (fp_) {
            Flush();
            fclose(fp_);
        }

        fp_ = fopen(file_name, "a");
        if (fp_ == nullptr) {
            mkdir(path_, 0777);
            fp_ = fopen(file_name, "a");
        }

        assert(fp_ != nullptr);
    }
}

Log* Log::Instance() {
    static Log instance;
    return &instance;
}

void Log::FlushLogThread() {
    Log::Instance()->AsyncWrite();
}

void Log::Write(int level, const char* format, ...) {
    // Get time
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    time_t t_sec = std::chrono::system_clock::to_time_t(now);

    struct tm* sys_time = localtime(&t_sec);
    struct tm t = *sys_time;
    va_list vas;

    // Date and lines in log file
    if (to_day_ != t.tm_mday ||
        (line_count_ && (line_count_ % MAX_LINES == 0))) {
        std::unique_lock<std::mutex> locker(mutex_);
        locker.unlock();

        char new_file[LOG_NAME_LEN];
        char tail[36] = {0};

        snprintf(tail, 36, "%04d_%02d_%02d",
                 t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (to_day_ != t.tm_mday) {
            snprintf(new_file, LOG_NAME_LEN - 72,
                     "%s/%s%s", path_, tail, suffix_);
            to_day_ = t.tm_mday;
            line_count_ = 0;
        } else {
            snprintf(new_file, LOG_NAME_LEN - 72,
                     "%s/%s%s", path_, tail, suffix_);
        }

        locker.lock();
        Flush();
        fclose(fp_);
        fp_ = fopen(new_file, "a");
        assert(fp_ != nullptr);
    }

    {
        std::unique_lock<std::mutex> locker(mutex_);
        ++line_count_;
        int n = snprintf(buffer_.BeginWrite(), 128,
                         "%d/%02d/%02d %02d:%02d:%02d ",
                         t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                         t.tm_hour, t.tm_min, t.tm_sec);
        buffer_.HasWritten(n);
        AppendLogLevelTitle(level);

        va_start(vas, format);
        int m = vsnprintf(buffer_.BeginWrite(), buffer_.WritableBytes(),
                          format, vas);
        va_end(vas);

        buffer_.HasWritten(m);
        buffer_.Append("\n\0", 2);

        if (is_async_ && deque_ && !deque_->Full()) {
            deque_->PushBack(buffer_.RetrieveAllToStr());
        } else {
            fputs(buffer_.Peek(), fp_);
        }

        buffer_.RetrieveAll();
    }
}

void Log::Flush() {
    if (is_async_) {
        deque_->Flush();
    }
    fflush(fp_);
}

int Log::GetLevel() {
    std::lock_guard<std::mutex> locker(mutex_);
    return level_;
}

void Log::SetLevel(int level) {
    std::lock_guard<std::mutex> locker(mutex_);
    level_ = level;
}

bool Log::IsOpen() const {
    return is_open_;
}

void Log::AppendLogLevelTitle(int level) {
    size_t title_len = 10;
    switch (level) {
        case 0:
            buffer_.Append("[DEBUG]   ", title_len);
            break;
        case 1:
            buffer_.Append("[INFO]    ", title_len);
            break;
        case 2:
            buffer_.Append("[WARNING] ", title_len);
            break;
        case 3:
            buffer_.Append("[ERROR]   ", title_len);
            break;
        default:
            buffer_.Append("[INFO]    ", title_len);
            break;
    }
}

void Log::AsyncWrite() {
    std::string str{};
    while (deque_->Pop(str)) {
        std::lock_guard<std::mutex> locker(mutex_);
        fputs(str.c_str(), fp_);
    }
}



