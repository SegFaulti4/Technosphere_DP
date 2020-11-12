#include "EPoll.h"

namespace net {

    void EPoll::ctl_(int op, int fd, int events) {
        ::epoll_event event{};
        event.events = events;
        event.data.fd = fd;

        if (::epoll_ctl(epoll_fd_.get_fd(), op, fd, &event) == -1) {
            switch(op) {
                case EPOLL_CTL_ADD:
                    throw NetException("Epoll add error");
                case EPOLL_CTL_DEL:
                    throw NetException("Epoll del error");
                case EPOLL_CTL_MOD:
                    throw NetException("Epoll mod error");
                default:
                    break;
            }
        }
    }

    EPoll::EPoll() {
        int epoll = epoll_create(1);
        if (epoll == -1) {
            throw NetException("Epoll create error");
        }
        epoll_fd_.set_fd(epoll);
    }

    void EPoll::add(int fd, Event_subscribe event) {
        ctl_(EPOLL_CTL_ADD, fd, event);
    }

    void EPoll::del(int fd) {
        ctl_(EPOLL_CTL_DEL, fd, 0);
    }

    void EPoll::mod(int fd, Event_subscribe event) {
        ctl_(EPOLL_CTL_MOD, fd, event);
    }

    int EPoll::wait(struct epoll_event *events, int max_events, int timeout) {
        int res = ::epoll_wait(epoll_fd_.get_fd(), events, max_events, timeout);
        if (res == -1) {
            throw NetException("Epoll wait error");
        }
        return res;
    }

}
