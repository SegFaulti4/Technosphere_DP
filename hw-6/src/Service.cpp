#include "Service.h"
#include <iostream>

namespace http {

    void Service::closeConnection_(int fd) {
        auto closed_con = connections_.find(fd);
        if (closed_con != connections_.end()) {
            connections_.erase(closed_con);
        }
    }

    Service::Service(unsigned worker_amount, std::unique_ptr<HttpServiceListener> listener) {
        worker_amount_ = worker_amount;
        listener_ = std::move(listener);
    }

    Service::~Service() {
        try {
            accept_epoll_.del(server_->get_descriptor().get_fd());
        } catch (std::exception &) {}
    }

    void Service::set_listener(std::unique_ptr<HttpServiceListener> listener) {
        listener_.reset(nullptr);
        listener_ = std::move(listener);
    }

    void Service::open(unsigned addr, unsigned port, int max_connection) {
        if (server_) {
            close();
        }
        server_ = std::make_unique<tcp::Server>(addr, port, max_connection);
        accept_epoll_.ctl_(EPOLL_CTL_ADD, server_->get_descriptor().get_fd(), EPOLLRDHUP | EPOLLIN);
    }

    void Service::open(const std::string & addr, unsigned port, int max_connection) {
        if (server_) {
            close();
        }
        server_ = std::make_unique<tcp::Server>(addr, port, max_connection);
        accept_epoll_.ctl_(EPOLL_CTL_ADD, server_->get_descriptor().get_fd(), EPOLLRDHUP | EPOLLIN);
    }

    void Service::close() {
        accept_epoll_.del(server_->get_descriptor().get_fd());
        server_.reset(nullptr);
    }


    void Service::run() {
        std::array<::epoll_event, event_queue_size> event_queue{};
        std::vector<std::thread> threads;
        threads.reserve(worker_amount_);
        for (int i = 0; i < worker_amount_; i++) {
            threads.emplace_back(&Service::worker_run_, this);
        }
        while (server_) {
            int events_count = accept_epoll_.wait(event_queue.data(), event_queue.size(), -1);
            for (int i = 0; i < events_count; i++) {
                if (event_queue[i].events & EPOLLIN) {
                    std::unique_ptr<HttpConnection> buf_con =
                            std::make_unique<HttpConnection>(server_->accept(), client_epoll_);
                    buf_con->subscribe(READWRITE);
                    int fd = buf_con->get_descriptor().get_fd();
                    connections_[fd] = std::move(buf_con);
                } else if (event_queue[i].events & EPOLLRDHUP) {
                    close();
                }
            }
        }
        for (int i = 0; i < worker_amount_; i++) {
            threads[i].join();
        }
    }

    void Service::worker_run_() {
        std::array<::epoll_event, event_queue_size> event_queue{};
        while (server_ || !connections_.empty()) {
            int events_count = client_epoll_.wait(event_queue.data(), event_queue.size(), 0);
            std::cout << "client event: " << events_count << std::endl;
            sleep(1);
            for (int i = 0; i < events_count; i++) {
                HttpConnection & buf_con = *connections_[event_queue[i].data.fd];
                if (event_queue[i].events & EPOLLRDHUP) {   // соединение закрыто клиентом, буфер чтения не парсится
                    std::cout << "RDHUP" << std::endl;
                    closeConnection_(event_queue[i].data.fd);
                } else if (event_queue[i].events & EPOLLIN) {
                    std::cout << "IN" << std::endl;
                    buf_con.read_until_eagain();
                    if (listener_) {
                        if (buf_con.request_available()) {
                            listener_->onRequest(buf_con);
                        }
                    }
                    buf_con.refresh_time();
                    buf_con.resubscribe();
                } else if (event_queue[i].events & EPOLLOUT) {
                    std::cout << "OUT" << std::endl;
                    if (buf_con.write_ongoing()) {
                        buf_con.write_until_eagain();
                        buf_con.refresh_time();
                        buf_con.resubscribe();
                    } else if (buf_con.downtime_duration() > max_downtime) {
                        closeConnection_(event_queue[i].data.fd);
                    } else {
                        buf_con.resubscribe();
                    }
                }
            }
        }
    }

    void Service::closeConnection(HttpConnection & buf_con) {
        closeConnection_(buf_con.get_descriptor().get_fd());
    }

}
