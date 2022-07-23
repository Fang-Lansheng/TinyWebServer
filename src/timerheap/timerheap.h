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


// Timer
struct TimerNode {
    // A unique identifier
    int id;

    // Expiration time of the timer
    TimeStamp expires;

    // The callback function, used to close the corresponding
    // HTTP connection when deleting the timer.
    TimeoutCallback callback;

    bool operator<(const TimerNode& t) const {
        return expires < t.expires;
    }
};


// A min-heap of timer.
class TimerHeap {
public:
    // ctor
    TimerHeap();

    // dtor
    ~TimerHeap();

    // Modify the timer specified by `id` and update its expiration time.
    void Adjust(int id, int timeout);

    // Add a timer.
    void Add(int id, int timeout, const TimeoutCallback& callback);

    // Remove a timer and trigger its callback function
    void Remove(int id);

    // Clear the heap.
    void Clear();

    // Clean the expired nodes
    void Tick();

    // Pop out the first node in the heap
    void Pop();

    // Returns the difference between the expiration time of the
    // connection at the top of heap and the current time.
    int GetNextTick();


private:
    // Delete an node
    void Delete(size_t index);

    // Shift up operation of heap.
    void ShiftUp(size_t index);

    // Shift down operation of heap.
    bool ShiftDown(size_t index, size_t n);

    // Swap two nodes.
    void SwapNode(size_t index1, size_t index2);

private:
    // The storage entity of the timers.
    std::vector<TimerNode> heap_;

    // The mapping from the timer to its position in the min-heap.
    std::unordered_map<int, size_t> ref_;
};


#endif //TINYWEBSERVER_SRC_TIMERHEAP_TIMERHEAP_H
