//
// Created by Jifan Zhang on 2022/4/11.
//

#ifndef TINYWEBSERVER_SRC_SERVER_EPOLLER_H
#define TINYWEBSERVER_SRC_SERVER_EPOLLER_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <vector>
#include <error.h>

class Epoller {
public:
    // Create an epoll instance and an events pool
    explicit Epoller(int max_event = 1024);

    // dtor
    ~Epoller();

    // Add the file descriptor `fd` to the interest list for `epoll_fd_`;
    // What we are interested in monitoring for `fd` is specified by `events`.
    // Returns `true` in case of success, `false` in case of error
    bool AddFd(int fd, uint32_t events);

    // Modify the events setting for the file descriptor `fd`, using the
    // information specified by `events`
    // Returns `true` in case of success, `false` in case of error
    bool ModFd(int fd, uint32_t events);

    // Remove the file descriptor `fd` from the interest list for `epoll_fd_`.
    // Returns `true` in case of success, `false` in case of error
    bool DelFd(int fd);

    // Wait for the first events in `events_`. The "timeout" parameter
    // specifies the maximum wait time in milliseconds (-1 == infinite).
    // Returns number of ready file descriptors, 0 on time out, or -1 on error.
    int Wait(int timeout = -1);

    // Get the file descriptor of the `index`-th epoll event
    int GetEventFd(size_t index) const;

    // Get the `index`-th epoll event
    uint32_t GetEvents(size_t index) const;

private:
    // An epoll instance (epoll file descriptor)
    int epoll_fd_;

    // A Buffer that contains triggered events
    std::vector<struct epoll_event> events_;
};


#endif //TINYWEBSERVER_SRC_SERVER_EPOLLER_H
