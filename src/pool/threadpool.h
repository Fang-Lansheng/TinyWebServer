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
    // Whether the thread pool is closed.
    bool is_closed;

    // The mutex variable.
    std::mutex mutex;

    // The conditional variable.
    std::condition_variable cond;

    // The queue of tasks to be "consumed".
    std::queue<std::function<void()>> tasks;
};

class ThreadPool {
public:
    // ctor
    explicit ThreadPool(size_t threat_count = 8);

    // ctor
    ThreadPool();

    // ctor
    ThreadPool(ThreadPool&&) noexcept ;

    // dtor
    ~ThreadPool();

    // Once a task is added, a thread is waked up to consume this task.
    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mutex);
            // add the task to the tasks queue.
            pool_->tasks.emplace(std::forward<F>(task));
        }

        // Wake up one thread.
        pool_->cond.notify_one();
    }

private:
    std::shared_ptr<Pool> pool_;
};


#endif //TINYWEBSERVER_SRC_POOL_THREADPOOL_H
