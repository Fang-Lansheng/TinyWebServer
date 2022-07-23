//
// Created by Jifan Zhang on 2022/4/11.
//

#include "epoller.h"

Epoller::Epoller(int max_event)
    : epoll_fd_(epoll_create(512)), events_(max_event) {
    assert(epoll_fd_ >= 0 && !events_.empty());
}

Epoller::~Epoller() {
    close(epoll_fd_);
}

bool Epoller::AddFd(int fd, uint32_t events) {
    if (fd < 0) {
        return false;
    }

    epoll_event event{};
    event.data.fd = fd;
    event.events = events;

    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
}

bool Epoller::ModFd(int fd, uint32_t events) {
    if (fd < 0) {
        return false;
    }

    epoll_event event{};
    event.data.fd = fd;
    event.events = events;

    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event);
}

bool Epoller::DelFd(int fd) {
    if (fd < 0) {
        return false;
    }

    epoll_event event{};

    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event);
}

int Epoller::Wait(int timeout) {
    return epoll_wait(epoll_fd_, &events_[0],
                      static_cast<int>(events_.size()), timeout);
}

int Epoller::GetEventFd(size_t index) const {
    assert(index < events_.size() && index >= 0);
    return events_[index].data.fd;
}

uint32_t Epoller::GetEvents(size_t index) const {
    assert(index < events_.size() && index >= 0);
    return events_[index].events;
}






