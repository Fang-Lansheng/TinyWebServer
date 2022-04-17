//
// Created by Jifan Zhang on 2022/4/13.
//

#ifndef TINYWEBSERVER_SRC_POOL_THREADPOOL_H
#define TINYWEBSERVER_SRC_POOL_THREADPOOL_H

#include <mutex>
#include <queue>
#include <thread>
#include <cassert>
#include <functional>
#include <condition_variable>

struct Pool {
    bool is_closed;
    std::mutex mutex;
    std::condition_variable cond;
    std::queue<std::function<void()>> tasks;
};

class ThreadPool {
public:
    explicit ThreadPool(size_t threat_count = 8);

    ThreadPool();

    ThreadPool(ThreadPool&&) noexcept ;

    ~ThreadPool();

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mutex);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();
    }

private:
    std::shared_ptr<Pool> pool_;
};


#endif //TINYWEBSERVER_SRC_POOL_THREADPOOL_H
