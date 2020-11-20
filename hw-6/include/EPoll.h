#ifndef HW_6_EPOLL_H
#define HW_6_EPOLL_H

#include <sys/epoll.h>
#include "tcp.h"
#include "HttpException.h"

namespace http {

    enum Event_subscribe {
        NOTHING = 0,
        READ = EPOLLIN,
        WRITE = EPOLLOUT,
        READWRITE = EPOLLIN | EPOLLOUT
    };

    class EPoll {
    private:
        tcp::Descriptor epoll_fd_;

        void ctl_(int op, int fd, int events);

    public:
        EPoll();

        void add(int fd, Event_subscribe event);
        void del(int fd);
        void mod(int fd, Event_subscribe event);
        int wait(struct epoll_event *events, int max_events, int timeout);

        friend class Service;
        friend class HttpConnection;
    };

}

#endif //HW_6_EPOLL_H
