#include "Service.h"

namespace net {

    void EPoll::ctl(int op, int fd, void * data, int events) {
        ::epoll_event event{};
        event.data.ptr = data;
        event.events = events;

        if (::epoll_ctl(epoll_fd_.getFd(), op, fd, &event) == -1) {
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
        epoll_fd_.setFd(epoll);
    }

    void EPoll::add(int fd, void * data, EventSubscribe event) {
        ctl(EPOLL_CTL_ADD, fd, data, event);
    }

    void EPoll::del(int fd, void * data) {
        ctl(EPOLL_CTL_DEL, fd, data, 0);
    }

    void EPoll::mod(int fd, void * data, EventSubscribe event) {
        ctl(EPOLL_CTL_MOD, fd, data, event);
    }

    int EPoll::wait(struct epoll_event *events, int max_events, int timeout) {
        int res = ::epoll_wait(epoll_fd_.getFd(), events, max_events, timeout);
        if (res == -1) {
            throw NetException("Epoll wait error");
        }
        return res;
    }

}
