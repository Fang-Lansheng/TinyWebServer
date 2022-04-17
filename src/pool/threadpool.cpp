//
// Created by Jifan Zhang on 2022/4/13.
//

#include "threadpool.h"

ThreadPool::ThreadPool(size_t threat_count):
    pool_(std::make_shared<Pool>()) {
    assert(threat_count > 0);
    for (size_t i = 0; i < threat_count; ++i) {
        // FIX IT: Initialized lambda captures are a C++14 extension
        std::thread([pool = pool_] {
            std::unique_lock<std::mutex> locker(pool->mutex);
            while (true) {
                if (!pool->tasks.empty()) {
                    auto task = std::move(pool->tasks.front());
                    pool->tasks.pop();
                    locker.unlock();
                    task();
                    locker.lock();
                } else if (pool->is_closed) {
                    break;
                } else {
                    pool->cond.wait(locker);
                }
            }
        }).detach();
    }
}

ThreadPool::~ThreadPool() {
    if (static_cast<bool>(pool_)) {
        {
            std::lock_guard<std::mutex> locker(pool_->mutex);
            pool_->is_closed = true;
        }
        pool_->cond.notify_all();
    }
}

ThreadPool::ThreadPool(ThreadPool &&) noexcept = default;

ThreadPool::ThreadPool() = default;




