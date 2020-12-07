#include "Service.h"
#include "NetException.h"

namespace net {

    void EPoll::ctl(int op, const tcp::Descriptor & fd, void * data, int events) {
        ::epoll_event event{};
        event.data.ptr = data;
        event.events = events;

        if (::epoll_ctl(epoll_fd_.getFd(), op, fd.getFd(), &event) == -1) {
            throw NetException("Epoll ctl ");
        }
    }

    EPoll::EPoll() {
        int epoll = ::epoll_create(1);
        if (epoll == -1) {
            throw NetException("Epoll create error");
        }
        epoll_fd_.setFd(epoll);
    }

    void EPoll::add(const tcp::Descriptor & fd, void * data, int event) {
        try {
            ctl(EPOLL_CTL_ADD, fd, data, event);
        } catch(NetException & exc) {
            throw NetException(std::string(exc.what()) + "add error");
        }
    }

    void EPoll::del(const tcp::Descriptor & fd, void * data) {
        try {
            ctl(EPOLL_CTL_DEL, fd, data, 0);
        } catch(NetException & exc) {
            throw NetException(std::string(exc.what()) + "del error");
        }
    }

    void EPoll::mod(const tcp::Descriptor & fd, void * data, int event) {
        try {
            ctl(EPOLL_CTL_MOD, fd, data, event);
        } catch(NetException & exc) {
            throw NetException(std::string(exc.what()) + "mod error");
        }
    }

    int EPoll::wait(struct epoll_event *events, int max_events, int timeout) {
        int res = ::epoll_wait(epoll_fd_.getFd(), events, max_events, timeout);
        if (res == -1) {
            throw NetException("Epoll wait error");
        }
        return res;
    }

}
