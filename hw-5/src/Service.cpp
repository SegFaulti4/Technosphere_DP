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

    void Service::set_listener(std::unique_ptr<IServiceListener> listener) {
        listener_.reset(nullptr);
        listener_ = std::move(listener);
    }

    void Service::open(unsigned addr, unsigned port, int max_connection) {
        server_ = std::make_unique<tcp::Server>(addr, port, max_connection);
        epoll_.ctl_(EPOLL_CTL_ADD, server_->get_descriptor().get_fd(), EPOLLRDHUP | EPOLLIN);
    }

    void Service::open(const std::string & addr, unsigned port, int max_connection) {
        server_ = std::make_unique<tcp::Server>(addr, port, max_connection);
        epoll_.ctl_(EPOLL_CTL_ADD, server_->get_descriptor().get_fd(), EPOLLRDHUP | EPOLLIN);
    }

    void Service::close() {
        epoll_.del(server_->get_descriptor().get_fd());
        server_.reset(nullptr);
    }

    void Service::run() {
        std::vector<::epoll_event> event_queue{1024};
        while(server_) {
            int server_fd = server_->get_descriptor().get_fd();
            int events_count = epoll_.wait(event_queue.data(), event_queue.size(), -1);
            auto start = event_queue.begin();
            auto end = event_queue.begin() + events_count;
            for (auto it = start; it != end; ++it) {
                if (it->data.fd == server_fd) {
                    std::unique_ptr<BufferedConnection> buf_con =
                            std::make_unique<BufferedConnection>(server_->accept(), epoll_);
                    int fd = buf_con->get_descriptor().get_fd();
                    connections_[fd] = std::move(buf_con);
                    if (listener_) {
                        listener_->onNewConnection(*connections_[fd]);
                    }
                } else if (it->events & EPOLLRDHUP) {
                    int fd = it->data.fd;
                    if (listener_) {
                        listener_->onClose(*connections_[fd]);
                    }
                    closeConnection_(fd);
                } else if (it->events & EPOLLIN) {
                    if (listener_) {
                        listener_->onReadAvailable(*connections_[it->data.fd]);
                    }
                } else if (it->events & EPOLLOUT) {
                    try {
                        connections_[it->data.fd]->buf_write();
                    } catch (tcp::TcpException & exc) {
                        if (listener_) {
                            listener_->onError(*connections_[it->data.fd], exc.what());
                        }
                    } catch (NetException & exc) {
                        if (listener_) {
                            listener_->onError(*connections_[it->data.fd], exc.what());
                        }
                    }
                    if (connections_[it->data.fd]->get_write_buf().empty()) {
                        if (listener_) {
                            listener_->onWriteDone(*connections_[it->data.fd]);
                        }
                    }
                }
            }
        }
    }

    void Service::closeConnection(BufferedConnection & buf_con) {
        closeConnection_(buf_con.get_descriptor().get_fd());
    }

    void Service::subscribeTo(BufferedConnection & buf_con, Event_subscribe event) {
        buf_con.subscribe(event);
    }

    void Service::unsubscribeFrom(BufferedConnection & buf_con, Event_subscribe event) {
        buf_con.unsubscribe(event);
    }

}
