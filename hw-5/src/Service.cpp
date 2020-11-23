#include "Service.h"

namespace net {

    void Service::closeConnection_(int fd) {
        auto closed_con = connections_.find(fd);
        if (closed_con != connections_.end()) {
            connections_.erase(closed_con);
        }
    }

    Service::Service(std::unique_ptr<IServiceListener> listener) {
        listener_ = std::move(listener);
    }

    Service::~Service() {
        try {
            epoll_.del(server_->get_descriptor().get_fd());
        } catch (std::exception &) {}
    }

    void Service::set_listener(std::unique_ptr<IServiceListener> listener) {
        listener_.reset(nullptr);
        listener_ = std::move(listener);
    }

    void Service::open(unsigned addr, unsigned port, int max_connection) {
        if (server_) {
            close();
        }
        server_ = std::make_unique<tcp::Server>(addr, port, max_connection);
        epoll_.ctl_(EPOLL_CTL_ADD, server_->get_descriptor().get_fd(), EPOLLRDHUP | EPOLLIN);
    }

    void Service::open(const std::string & addr, unsigned port, int max_connection) {
        if (server_) {
            close();
        }
        server_ = std::make_unique<tcp::Server>(addr, port, max_connection);
        epoll_.ctl_(EPOLL_CTL_ADD, server_->get_descriptor().get_fd(), EPOLLRDHUP | EPOLLIN);
    }

    void Service::close() {
        epoll_.del(server_->get_descriptor().get_fd());
        server_.reset(nullptr);
    }

    void Service::run() {
        std::array<::epoll_event, event_queue_size_> event_queue{};
        while (server_ || !connections_.empty()) {
            int server_fd = server_->get_descriptor().get_fd();
            int events_count = epoll_.wait(event_queue.data(), event_queue.size(), -1);
            for (int i = 0; i < events_count; i++) {
                if (event_queue[i].data.fd == server_fd) {
                    if (event_queue[i].events & EPOLLIN) {
                        std::unique_ptr<BufferedConnection> buf_con =
                                std::make_unique<BufferedConnection>(server_->accept(), epoll_, *this);
                        int fd = buf_con->get_descriptor().get_fd();
                        connections_[fd] = std::move(buf_con);
                        if (listener_) {
                            listener_->onNewConnection(*connections_[fd]);
                        }
                    } else if (event_queue[i].events & EPOLLRDHUP) {
                        close();
                    }
                } else if (event_queue[i].events & EPOLLRDHUP) {
                    int fd = event_queue[i].data.fd;
                    if (listener_) {
                        listener_->onClose(*connections_[fd]);
                    }
                    closeConnection_(fd);
                } else if (event_queue[i].events & EPOLLIN) {
                    if (listener_) {
                        listener_->onReadAvailable(*connections_[event_queue[i].data.fd]);
                    }
                } else if (event_queue[i].events & EPOLLOUT) {
                    if (!connections_[event_queue[i].data.fd]->get_write_buf().empty()) {
                        try {
                            connections_[event_queue[i].data.fd]->buf_write();
                        } catch (tcp::TcpException &exc) {
                            if (listener_) {
                                listener_->onError(*connections_[event_queue[i].data.fd], exc.what());
                            }
                        } catch (NetException &exc) {
                            if (listener_) {
                                listener_->onError(*connections_[event_queue[i].data.fd], exc.what());
                            }
                        }
                        if (connections_[event_queue[i].data.fd]->get_write_buf().empty()) {
                            if (listener_) {
                                listener_->onWriteDone(*connections_[event_queue[i].data.fd]);
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

}
