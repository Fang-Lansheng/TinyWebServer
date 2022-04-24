//
// Created by Jifan Zhang on 2022/4/24.
//

#include <features.h>

#include "../src/log/log.h"
#include "../src/pool/threadpool.h"

#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

void TestLog() {
    int count = 0, level = 0;
    Log::Instance()->Init(level, "./logs/test_log_1", ".log", 0);
    for (level = 3; level >= 0; --level) {
        Log::Instance()->SetLevel(level);
        for (int j = 0; j < 10000; ++j) {
            for (int i = 0; i < 4; ++i) {
                LOG_BASE(i, "%s 1111111111 %d ========== ", "Test", count++);
            }
        }
    }

    count = 0;
    Log::Instance()->Init(level, "./logs/test_log_2", ".log", 5000);
    for (level = 0; level < 4; ++level) {
        Log::Instance()->SetLevel(level);
        for (int j = 0; j < 10000; ++j) {
            for (int i = 0; i < 4; ++i) {
                LOG_BASE(level, "%s 2222222222 %d ========== ", "Test", count++);
            }
        }
    }
}


void ThreadLogTask(int i, int count) {
    for (int j = 0; j < 10000; ++j) {
        LOG_BASE(i, "PID: [%04d] ========== %05d ========== ", gettid(), count++);
    }
}

void TestThreadPool() {
    Log::Instance()->Init(0, "./logs/test_thread_pool", ".log", 5000);
    ThreadPool thread_pool(6);
    for (int i = 0; i < 18; ++i) {
        thread_pool.AddTask(std::bind(ThreadLogTask, i % 4, i * 10000));
    }
    getchar();
}

int main() {
    TestLog();
    TestThreadPool();
}
