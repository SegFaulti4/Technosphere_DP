#include "Service.h"

namespace net {

    void EPoll::ctl(int op, Epoll_data * data, int events) {
        ::epoll_event event{};
        event.events = events;
        event.data.ptr = data;

        if (::epoll_ctl(epoll_fd_.get_fd(), op, data->fd, &event) == -1) {
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

    void EPoll::add(Epoll_data * data, Event_subscribe event) {
        ctl(EPOLL_CTL_ADD, data, event);
    }

    void EPoll::del(Epoll_data * data) {
        ctl(EPOLL_CTL_DEL, data, 0);
    }

    void EPoll::mod(Epoll_data * data, Event_subscribe event) {
        ctl(EPOLL_CTL_MOD, data, event);
    }

    int EPoll::wait(struct epoll_event *events, int max_events, int timeout) {
        int res = ::epoll_wait(epoll_fd_.get_fd(), events, max_events, timeout);
        if (res == -1) {
            throw NetException("Epoll wait error");
        }
        return res;
    }

}
