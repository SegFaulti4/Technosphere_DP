#include "Service.h"

namespace net {

    void Service::closeConnection_(int fd) {
        auto closed_con = connections_.find(fd);
        if (closed_con != connections_.end()) {
            connections_.erase(closed_con);
        }
    }

    Service::Service(IServiceListener * listener) {
        epoll_data_.type = SERVER;
        epoll_data_.ptr = nullptr;
        listener_ = listener;
    }

    void Service::setListener(IServiceListener * listener) {
        listener_ = listener;
    }

    void Service::open(unsigned addr, unsigned port, int max_connection) {
        server_.listen(addr, port, max_connection);
        epoll_data_.fd = server_.get_descriptor().get_fd();
        epoll_.ctl(EPOLL_CTL_ADD, &epoll_data_, EPOLLRDHUP | EPOLLIN);
    }

    void Service::open(const std::string & addr, unsigned port, int max_connection) {
        server_.listen(addr, port, max_connection);
        epoll_data_.fd = server_.get_descriptor().get_fd();
        epoll_.ctl(EPOLL_CTL_ADD, &epoll_data_, EPOLLRDHUP | EPOLLIN);
    }

    void Service::close() {
        setRunning(false);
        epoll_.del(&epoll_data_);
        server_.close();
    }

    void Service::run() {
        if (!isRunning()) {
            std::array<::epoll_event, event_queue_size_> event_queue{};
            setRunning(true);
            while (isRunning()) {
                int server_fd = server_.get_descriptor().get_fd();
                int events_count = epoll_.wait(event_queue.data(), event_queue.size(), -1);
                for (int i = 0; i < events_count; i++) {
                    auto epoll_data = reinterpret_cast<Epoll_data *>(event_queue[i].data.ptr);
                    if (epoll_data->type == SERVER) {
                        if (event_queue[i].events & EPOLLIN) {
                            if (listener_) {
                                listener_->onNewConnection(std::move(BufferedConnection
                                        (server_.accept(), epoll_)), *this);
                            }
                        } else if (event_queue[i].events & EPOLLRDHUP) {
                            close();
                        }
                    } else if (event_queue[i].events & EPOLLRDHUP) {
                        int fd = epoll_data->fd;
                        if (listener_) {
                            listener_->onClose(connections_.at(fd), *this);
                        }
                        closeConnection_(fd);
                    } else if (event_queue[i].events & EPOLLIN) {
                        if (listener_) {
                            listener_->onReadAvailable(connections_.at(epoll_data->fd), *this);
                        }
                    } else if (event_queue[i].events & EPOLLOUT) {
                        if (!connections_.at(epoll_data->fd).get_write_buf().empty()) {
                            try {
                                connections_.at(epoll_data->fd).read_into_buf();
                            } catch (tcp::TcpException &exc) {
                                if (listener_) {
                                    listener_->onError(connections_.at(epoll_data->fd), exc.what(), *this);
                                }
                            }
                            if (connections_.at(epoll_data->fd).get_write_buf().empty()) {
                                if (listener_) {
                                    listener_->onWriteDone(connections_.at(epoll_data->fd), *this);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void Service::closeConnection(BufferedConnection & buf_con) {
        closeConnection_(buf_con.get_descriptor().get_fd());
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
        connections_.emplace(buf_con.get_descriptor().get_fd(), std::move(buf_con));
    }

}
