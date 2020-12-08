#include "Service.h"
#include <memory>
#include "NetException.h"

namespace net {

    constexpr size_t EVENT_QUEUE_SIZE = 1024;

    void Service::closeConnection_(int fd) {
        auto closed_con = connections_.find(fd);
        if (closed_con != connections_.end()) {
            connections_.erase(closed_con);
        }
    }

    Service::Service(IServiceListener * listener) {
        epoll_data_ = &server_;
        listener_ = listener;
    }

    void Service::setListener(IServiceListener * listener) {
        listener_ = listener;
    }

    void Service::open(unsigned addr, unsigned port, int max_connection) {
        server_.listen(addr, port, max_connection);
        epoll_.add(server_.getDescriptor(), epoll_data_, EPOLLRDHUP | EPOLLIN);
    }

    void Service::open(const std::string & addr, unsigned port, int max_connection) {
        server_.listen(addr, port, max_connection);
        epoll_.add(server_.getDescriptor(), epoll_data_, EPOLLRDHUP | EPOLLIN);
    }

    void Service::close() {
        setRunning(false);
        epoll_.del(server_.getDescriptor(), epoll_data_);
        server_.close();
    }

    void Service::run() {
        if (!isRunning()) {
            std::array<::epoll_event, EVENT_QUEUE_SIZE> event_queue{};
            setRunning(true);
            while (isRunning()) {
                int server_fd = server_.getDescriptor().getFd();
                int events_count = epoll_.wait(event_queue.data(), event_queue.size(), -1);
                for (int i = 0; i < events_count; i++) {
                    if (event_queue[i].data.ptr == &server_) {
                        if (event_queue[i].events & EPOLLIN) {
                            if (listener_) {
                                listener_->onNewConnection(std::move(BufferedConnection
                                        (server_.accept(), epoll_)), *this);
                            }
                        } else if (event_queue[i].events & EPOLLRDHUP) {
                            close();
                        }
                    } else {
                        int fd = reinterpret_cast<BufferedConnection *>(event_queue[i].data.ptr)
                                    ->getDescriptor().getFd();
                        if (event_queue[i].events & EPOLLRDHUP) {
                            if (listener_) {
                                listener_->onClose(connections_.at(fd), *this);
                            }
                            closeConnection_(fd);
                        } else if (event_queue[i].events & EPOLLIN) {
                            if (listener_) {
                                listener_->onReadAvailable(connections_.at(fd), *this);
                            }
                        } else if (event_queue[i].events & EPOLLOUT) {
                            if (!connections_.at(fd).getWriteBuf().empty()) {
                                try {
                                    connections_.at(fd).readIntoBuf();
                                } catch (tcp::TcpException &exc) {
                                    if (listener_) {
                                        listener_->onError(connections_.at(fd), exc.what(), *this);
                                    }
                                }
                                if (connections_.at(fd).getWriteBuf().empty()) {
                                    if (listener_) {
                                        listener_->onWriteDone(connections_.at(fd), *this);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void Service::closeConnection(BufferedConnection & buf_con) {
        closeConnection_(buf_con.getDescriptor().getFd());
    }

    void Service::setRunning(bool b) {
        running_ = b;
    }

    bool Service::isRunning() const {
        return running_;
    }

    Service & Service::operator=(Service && other)  noexcept {
        listener_ = other.listener_;
        connections_ = std::move(other.connections_);
        server_ = std::move(other.server_);
        epoll_ = std::move(other.epoll_);
        return *this;
    }

    void Service::addConnection(BufferedConnection && buf_con) {
        connections_.emplace(buf_con.getDescriptor().getFd(), std::move(buf_con));
    }

}
