//
// Created by Jifan Zhang on 2022/4/13.
//

#include "timerheap.h"

TimerHeap::TimerHeap() {
    heap_.reserve(64);
}

TimerHeap::~TimerHeap() {
    Clear();
}

void TimerHeap::Adjust(int id, int timeout) {
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);
    ShiftDown(ref_[id], heap_.size());
}

void TimerHeap::Add(int id, int timeout, const TimeoutCallback &callback) {
    assert(id >= 0);
    size_t i;

    if (!ref_.count(id)) {
        // If node `id` not exist, insert it into `heap_`
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeout), callback});
        ShiftUp(i);
    } else {
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeout);
        heap_[i].callback = callback;
        if (!ShiftDown(i, heap_.size())) {
            ShiftUp(i);
        }
    }
}

void TimerHeap::Remove(int id) {
    if (heap_.empty() || !ref_.count(id)) {
        return;
    }

    size_t index = ref_[id];
    TimerNode node = heap_[index];
    node.callback();
    Delete(index);
}

void TimerHeap::Clear() {
    ref_.clear();
    heap_.clear();
}

void TimerHeap::Tick() {
    if (heap_.empty()) {
        return;
    }
    while (!heap_.empty()) {
        TimerNode node = heap_.front();
        if (std::chrono::duration_cast<MS>(
                node.expires - Clock::now()).count() > 0) {
            break;
        } else {
            node.callback();
            Pop();
        }
    }
}

void TimerHeap::Pop() {
    assert(!heap_.empty());
    Delete(0);
}

int TimerHeap::GetNextTick() {
    Tick();
    if (!heap_.empty()) {
        auto res = std::chrono::duration_cast<MS>(
                heap_.front().expires - Clock::now()).count();
        if (res < 0) {
            res = 0;
        }
        return static_cast<int>(res);
    }
    return -1;
}

// ----- private -----

void TimerHeap::Delete(size_t index) {
    // Move the node `index` to the end of the heap, and then
    // adjust the heap.
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    size_t i = index;
    size_t n = heap_.size() - 1;

    if (i < n) {
        SwapNode(i, n);
        if (!ShiftDown(i, n)) {
            ShiftUp(i);
        }
    }

    // Remove the node at the end of TimerHeap
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void TimerHeap::ShiftUp(size_t index) {
    assert(index >= 0 && index < heap_.size());
    if (index == 0) {
        return;
    } else {
        size_t temp = (index - 1) / 2;
        while (temp >= 0) {
            if (heap_[temp] < heap_[index]) {
                break;
            }
            SwapNode(index, temp);
            index = temp;
            temp = (index - 1) / 2;
        }
    }
}

bool TimerHeap::ShiftDown(size_t index, size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i * 2 + 1;
    while (j < n) {
        if (j + 1 < n && heap_[j + 1] < heap_[j]) {
            ++j;
        }
        if (heap_[i] < heap_[j]) {
            break;
        }
        SwapNode(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void TimerHeap::SwapNode(size_t index1, size_t index2) {
    assert(index1 >= 0 && index1 < heap_.size());
    assert(index2 >= 0 && index2 < heap_.size());

    std::swap(heap_[index1], heap_[index2]);
    ref_[heap_[index1].id] = index1;
    ref_[heap_[index2].id] = index2;
}


