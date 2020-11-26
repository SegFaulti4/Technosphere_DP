#ifndef NET_EPOLL_H
#define NET_EPOLL_H

#include <sys/epoll.h>
#include <mutex>
#include <memory>
#include "tcp.h"
#include "NetException.h"

namespace net {

    enum Event_subscribe {
        NOTHING = 0,
        READ = EPOLLIN,
        WRITE = EPOLLOUT,
        READWRITE = EPOLLIN | EPOLLOUT
    };

    enum Net_ptr_type {
        SERVER = 0,
        CONNECTION = 1
    };

    typedef struct Epoll_data {
        Net_ptr_type type;
        void * ptr;
        int meta;
        int fd;
    } Epoll_data;

    class EPoll {
    private:
        tcp::Descriptor epoll_fd_;

    public:
        EPoll();

        void add(Epoll_data * data, Event_subscribe event);
        void del(Epoll_data * data);
        void mod(Epoll_data * data, Event_subscribe event);
        int wait(struct epoll_event *events, int max_events, int timeout);
        void ctl(int op, Epoll_data * data, int events);
    };

}

#endif //NET_EPOLL_H
