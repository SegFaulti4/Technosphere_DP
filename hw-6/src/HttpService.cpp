#include <iostream>
#include "HttpService.h"

namespace http {

    HttpService::HttpService(unsigned worker_amount, IHttpServiceListener * listener) {
        worker_amount_ = worker_amount;
        listener_ = listener;
        epoll_data_.type = net::SERVER;
        epoll_data_.meta = 0;
        epoll_data_.ptr = nullptr;
    }

    void HttpService::setListener(IHttpServiceListener * listener) {
        listener_ = listener;
    }

    void HttpService::open(unsigned addr, unsigned port, int max_connection) {
        server_.listen(addr, port, max_connection);
        epoll_data_.fd = server_.get_descriptor().get_fd();
        accept_epoll_.ctl(EPOLL_CTL_ADD, &epoll_data_, EPOLLRDHUP | EPOLLIN);
    }

    void HttpService::open(const std::string & addr, unsigned port, int max_connection) {
        server_.listen(addr, port, max_connection);
        epoll_data_.fd = server_.get_descriptor().get_fd();
        accept_epoll_.ctl(EPOLL_CTL_ADD, &epoll_data_, EPOLLRDHUP | EPOLLIN);
    }

    void HttpService::close() {
        setRunning(false);
        accept_epoll_.del(&epoll_data_);
        server_.close();
    }

    void HttpService::run() {
        if (!isRunning()) {
            std::array<::epoll_event, event_queue_size> event_queue{};
            std::vector<std::thread> threads;
            threads.reserve(worker_amount_);
            for (int i = 0; i < worker_amount_; i++) {
                threads.emplace_back(&http::HttpService::workerRun_, this);
            }
            watchdog_time_point_ = steady_clock::now();
            while (isRunning()) {
                if (watchdog_time_() > watchdog_period) {
                    watchdog_();
                }
                int events_count = accept_epoll_.wait(event_queue.data(), event_queue.size(), -1);
                for (int i = 0; i < events_count; i++) {
                    auto epoll_data = reinterpret_cast<net::Epoll_data *>(event_queue[i].data.ptr);
                    if (epoll_data->type == net::SERVER && event_queue[i].events & EPOLLIN) {
                        int j = findNewMutex_();
                        if (j == -1) {
                            throw HttpException("Maximum connection amount reached");
                        }
                        addConnection_(std::move(HttpConnection(net::BufferedConnection
                                (server_.accept(), client_epoll_), i)));
                    } else {
                        throw HttpException("Invalid event in service run");
                    }
                }
            }
            for (std::thread& t : threads) {
                t.join();
            }
        }
    }

    void HttpService::closeConnection_(HttpConnection & http_con) {
        closeConnection_(http_con.getDescriptor().get_fd());
    }

    void HttpService::addConnection_(HttpConnection && http_con) {
        connections_.emplace(http_con.getDescriptor().get_fd(), std::move(http_con));
    }

    void HttpService::setRunning(bool b) {
        running_ = b;
    }

    bool HttpService::isRunning() const {
        return running_;
    }

    void HttpService::closeConnection_(int fd) {
        auto closed_con = connections_.find(fd);
        if (closed_con != connections_.end()) {
            connections_.erase(closed_con);
        } else {
            throw HttpException("Erasing invalid HttpConnection");
        }
    }

    void HttpService::workerRun_() {
        std::array<::epoll_event, event_queue_size> event_queue{};
        while (isRunning() || !connections_.empty()) {
            int events_count = client_epoll_.wait(event_queue.data(), event_queue.size(), 0);
            std::cerr << "client event: " << events_count << std::endl;
            sleep(1);
            for (int i = 0; i < events_count; i++) {
                auto epoll_data = reinterpret_cast<net::Epoll_data *>(event_queue[i].data.ptr);
                mutexes[epoll_data->meta].lock();
                if (used_mutexes[epoll_data->meta]) {
                    if (epoll_data->type != net::CONNECTION) {
                        throw HttpException("Invalid event in client run");
                    }
                    HttpConnection & http_con = *reinterpret_cast<HttpConnection *>(epoll_data->ptr);
                    if (event_queue[i].events & EPOLLRDHUP) {   // соединение закрыто клиентом, буфер чтения не парсится
                        std::cerr << "RDHUP" << std::endl;
                        http_con.set_valid(false);
                    } else {
                        if (event_queue[i].events & EPOLLOUT) {
                            std::cerr << "OUT" << std::endl;
                            http_con.write_until_eagain();
                            if (!http_con.write_ongoing()) {
                                http_con.unsubscribe(net::WRITE);
                                http_con.refresh_time();
                            } else {
                                http_con.resubscribe();
                            }
                        }
                        if (event_queue[i].events & EPOLLIN) {
                            std::cerr << "IN" << std::endl;
                            http_con.read_until_eagain();
                            if (listener_) {
                                if (http_con.request_available()) {
                                    bool keep_alive = false;
                                    HttpRequest &request = http_con.get_request();
                                    if (request.find("Connection") != request.end()
                                        && request["Connection"] == "keep-alive") {
                                        keep_alive = true;
                                    }
                                    listener_->onRequest(http_con);
                                    if (!keep_alive) {
                                        http_con.set_valid(false);
                                    }
                                }
                            }
                            if (http_con.is_valid()) {
                                http_con.resubscribe();
                            }
                        }
                    }
                }
                mutexes[epoll_data->meta].unlock();
            }
        }
    }

    int HttpService::findNewMutex_() {
        static int i = 0;
        int old_i = i;
        for (; i < max_connection_amount; i++) {
            if (!used_mutexes[i]) {
                return i++;
            }
        }
        for (i = 0; i < old_i; i++) {
            if (!used_mutexes[i]) {
                return i++;
            }
        }
        return -1;
    }

    int HttpService::watchdog_time_() {
        return std::chrono::duration_cast<ms>(steady_clock::now() - watchdog_time_point_).count();
    }

    void HttpService::watchdog_() {
        for (auto & connection : connections_) {
            int mutex_idx = connection.second.get_mutex_idx();
            if (!used_mutexes[mutex_idx]) {
                throw HttpException("Invalid mutex");
            }
            mutexes[mutex_idx].lock();
            if (connection.second.downtime_duration() > max_downtime || !connection.second.is_valid()) {
                closeConnection_(connection.second);
                used_mutexes[mutex_idx] = false;
            }
            mutexes[mutex_idx].unlock();
        }
    }

}
