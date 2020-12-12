#ifndef NET_EPOLL_H
#define NET_EPOLL_H

#include <sys/epoll.h>
#include "tcp.h"

namespace net {

    class EPoll {
    private:
        tcp::Descriptor epoll_fd_;
        void ctl(int op, const tcp::Descriptor & fd, void * data, int events);

    public:
        EPoll();

        void add(const tcp::Descriptor & fd, void * data, int event);
        void del(const tcp::Descriptor & fd, void * data);
        void mod(const tcp::Descriptor & fd, void * data, int event);
        int wait(struct epoll_event *events, int max_events, int timeout);

        friend class BufferedConnection;
    };

}

#endif //NET_EPOLL_H
