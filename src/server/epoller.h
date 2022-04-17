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
    // Create an epoll instance and a events pool
    explicit Epoller(int max_event = 1024);

    ~Epoller();

    // Add an epoll install `fd`.
    // Returns 0 in case of success, -1 in case of error
    bool AddFd(int fd, uint32_t events);

    // Modify an epoll instance `fd`.
    // Returns 0 in case of success, -1 in case of error
    bool ModFd(int fd, uint32_t events);

    // Modify an epoll instance `fd`.
    // Returns 0 in case of success, -1 in case of error
    bool DelFd(int fd);

    // Wait for the first events in `events_`.
    // The "timeout" parameter specifies the maximum wait time in
    // milliseconds (-1 == infinite).
    // Returns the number of triggered events returned in `events_`
    // Buffer. Or -1 in case of error with the "errno" variable set
    // to the specific error code.
    int Wait(int timeout = -1);

    // Get the `index`-th epoll event's fd
    int GetEventFd(size_t index) const;

    // Get the `index`-th epoll event's
    uint32_t GetEvents(size_t index) const;

private:
    // An epoll instance
    int epoll_fd_;

    // A Buffer that contains triggered events
    std::vector<struct epoll_event> events_;
};


#endif //TINYWEBSERVER_SRC_SERVER_EPOLLER_H
