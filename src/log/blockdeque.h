//
// Created by Jifan Zhang on 2022/4/11.
//

#ifndef TINYWEBSERVER_SRC_LOG_BLOCKDEQUE_H
#define TINYWEBSERVER_SRC_LOG_BLOCKDEQUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <ctime>
#include <cassert>

template<class T> class BlockDeque {
public:
    explicit BlockDeque(size_t max_capacity = 1000);

    ~BlockDeque();

    void Clear();

    bool Empty();

    bool Full();

    void Close();

    size_t Size();

    size_t Capacity();

    T Front();

    T Back();

    void PushBack(const T& item);

    void PushFront(const T& item);

    bool Pop(T& item);

    bool Pop(T& item, int timeout);

    void Flush();

private:
    std::deque<T> deque_;
    size_t capacity_;
    std::mutex mutex_;
    bool is_close_{};
    std::condition_variable condition_consumer_;
    std::condition_variable condition_producer_;
};

template<class T>
BlockDeque<T>::BlockDeque(size_t max_capacity): capacity_(max_capacity) {
    assert(max_capacity > 0);
}

template<class T>
BlockDeque<T>::~BlockDeque() {
    Close();
}

template<class T>
void BlockDeque<T>::Clear() {
    std::lock_guard<std::mutex> locker(mutex_);
    deque_.clear();
}

template<class T>
bool BlockDeque<T>::Empty() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deque_.empty();
}

template<class T>
bool BlockDeque<T>::Full() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deque_.size() >= capacity_;
}

template<class T>
void BlockDeque<T>::Close() {
    {
        std::lock_guard<std::mutex> locker(mutex_);
        deque_.clear();
        is_close_ = true;
    }

    condition_consumer_.notify_all();
    condition_producer_.notify_all();
}

template<class T>
size_t BlockDeque<T>::Size() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deque_.size();
}

template<class T>
size_t BlockDeque<T>::Capacity() {
    std::lock_guard<std::mutex> locker(mutex_);
    return capacity_;
}

template<class T>
T BlockDeque<T>::Front() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deque_.front();
}

template<class T>
T BlockDeque<T>::Back() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deque_.back();
}

template<class T>
void BlockDeque<T>::PushBack(const T &item) {
    std::unique_lock<std::mutex> locker(mutex_);
    while (deque_.size() >= capacity_) {
        condition_producer_.wait(locker);
    }
    deque_.push_back(item);
    condition_consumer_.notify_one();
}

template<class T>
void BlockDeque<T>::PushFront(const T &item) {
    std::unique_lock<std::mutex> locker(mutex_);
    while (deque_.size() >= capacity_) {
        condition_producer_.wait(locker);
    }
    deque_.push_front(item);
    condition_consumer_.notify_one();
}

template<class T>
bool BlockDeque<T>::Pop(T &item) {
    std::unique_lock<std::mutex> locker(mutex_);
    while (deque_.empty()) {
        condition_consumer_.wait(locker);
        if (is_close_) {
            return false;
        }
    }
    item = deque_.front();
    deque_.pop_front();
    condition_producer_.notify_one();
    return true;
}

template<class T>
bool BlockDeque<T>::Pop(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mutex_);
    while (deque_.empty()) {
        if (condition_consumer_.wait_for(
                locker, std::chrono::seconds(timeout)) ==
                std::cv_status::timeout) {
            return false;
        }
        if (is_close_) {
            return false;
        }
    }

    item = deque_.front();
    deque_.pop_front();
    condition_producer_.notify_one();
    return true;
}

template<class T>
void BlockDeque<T>::Flush() {
    condition_consumer_.notify_one();
}

#endif //TINYWEBSERVER_SRC_LOG_BLOCKDEQUE_H
