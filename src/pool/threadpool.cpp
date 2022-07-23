//
// Created by Jifan Zhang on 2022/4/13.
//

#include "threadpool.h"

ThreadPool::ThreadPool(size_t threat_count): pool_(std::make_shared<Pool>()) {
    assert(threat_count > 0);
    for (size_t i = 0; i < threat_count; ++i) {
        std::thread([pool = pool_] {
            // Get the locker of the thread pool
            std::unique_lock<std::mutex> locker(pool->mutex);

            // The tasks in the queue are processed in sequence
            // until the thread pool is closed.
            while (true) {
                if (!pool->tasks.empty()) {
                    // Fetch a task from the head of the queue.
                    auto task = std::move(pool->tasks.front());
                    pool->tasks.pop();

                    // Process the task.
                    locker.unlock();
                    task();
                    locker.lock();
                } else if (pool->is_closed) {
                    break;
                } else {
                    // keep waiting if the tasks queue is empty.
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

ThreadPool::ThreadPool(ThreadPool&&) noexcept = default;

ThreadPool::ThreadPool() = default;




