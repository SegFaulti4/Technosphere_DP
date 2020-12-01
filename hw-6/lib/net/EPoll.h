#ifndef NET_EPOLL_H
#define NET_EPOLL_H

#include <sys/epoll.h>
#include <mutex>
#include <memory>
#include "tcp.h"
#include "NetException.h"

namespace net {

    enum EventSubscribe {
        NOTHING = 0,
        READ = EPOLLIN,
        WRITE = EPOLLOUT,
        READWRITE = EPOLLIN | EPOLLOUT
    };

    class EPoll {
    private:
        tcp::Descriptor epoll_fd_;

    public:
        EPoll();

        void add(int fd, void * data, EventSubscribe event);
        void del(int fd, void * data);
        void mod(int fd, void * data, EventSubscribe event);
        int wait(struct epoll_event *events, int max_events, int timeout);
        void ctl(int op, int fd, void * data, int events);
    };

}

#endif //NET_EPOLL_H
