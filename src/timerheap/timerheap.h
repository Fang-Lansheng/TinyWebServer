//
// Created by Jifan Zhang on 2022/4/13.
//

#ifndef TINYWEBSERVER_SRC_TIMERHEAP_TIMERHEAP_H
#define TINYWEBSERVER_SRC_TIMERHEAP_TIMERHEAP_H

#include <queue>
#include <unordered_map>
#include <ctime>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <cassert>
#include <chrono>

#include "../log/log.h"

typedef std::function<void()> TimeoutCallback;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;


struct TimerNode {
    int                 id;
    TimeStamp           expires;
    TimeoutCallback     callback;

    bool operator<(const TimerNode& t) const {
        return expires < t.expires;
    }
};


class TimerHeap {
public:
    TimerHeap();

    ~TimerHeap();

    // Modify a `TimerNode`
    void Adjust(int id, int timeout);

    // Add a `TimerNode`
    void Add(int id, int timeout, const TimeoutCallback& callback);

    // Remove a `TimerNode` and trigger the callback function
    void Remove(int id);

    // Clear the heap
    void Clear();

    // Clean the expired nodes
    void Tick();

    // Pop out the first node in the heap
    void Pop();

    int GetNextTick();


private:
    // Delete an node
    void Delete(size_t index);

    void ShiftUp(size_t index);

    bool ShiftDown(size_t index, size_t n);

    // Swap two nodes
    void SwapNode(size_t index1, size_t index2);

    std::vector<TimerNode> heap_;

    std::unordered_map<int, size_t> ref_;
};


#endif //TINYWEBSERVER_SRC_TIMERHEAP_TIMERHEAP_H
