#include <iostream>
#include "HttpService.h"

namespace http {

    constexpr size_t event_queue_size = 1024;
    constexpr int max_downtime = 2000;
    constexpr int max_connection_amount = 10000;
    constexpr int watchdog_period = 10;

    HttpService::HttpService(unsigned worker_amount, IHttpServiceListener * listener) {
        worker_amount_ = worker_amount;
        listener_ = listener;
    }

    void HttpService::setListener(IHttpServiceListener * listener) {
        listener_ = listener;
    }

    void HttpService::open(unsigned addr, unsigned port, int max_connection) {
        server_.listen(addr, port, max_connection);
    }

    void HttpService::open(const std::string & addr, unsigned port, int max_connection) {
        server_.listen(addr, port, max_connection);
    }

    void HttpService::close() {
        setRunning_(false);
        server_.close();
    }

    void HttpService::run() {
        if (!isRunning()) {
            std::vector<std::thread> threads;
            threads.reserve(worker_amount_);
            for (int i = 0; i < worker_amount_; i++) {
                threads.emplace_back(&http::HttpService::workerRun_, this);
            }
            setRunning_(true);
            server_.setTimeout(watchdog_period);
            while (isRunning()) {
                if (opened_connections_amount_ < max_connection_amount) {
                    try {
                        HttpConnection http_con(net::BufferedConnection(server_.accept(), client_epoll_));
                        log::info("New connection");
                        opened_connections_amount_++;
                        if (available_connections_.empty()) {
                            log::info("No connections available");
                            addConnection_(std::move(http_con));
                        } else {
                            {
                                std::lock_guard lock(queue_mutex_);
                                auto con = available_connections_.front();
                                *con = std::move(http_con);
                                con->openEpoll();
                                available_connections_.pop();
                            }
                        }
                    } catch (tcp::TcpBlockException &) {
                        log::debug("Accept would block");
                    } catch (std::exception &exc) {
                        throw exc;
                    }
                } else {
                    log::warn("Max connection amount reached");
                    std::this_thread::sleep_for(std::chrono::milliseconds(watchdog_period));
                }
                if (watchdogDowntimeDuration_() > watchdog_period) {
                    watchdog_();
                }
            }
            for (std::thread& t : threads) {
                t.join();
            }
        }
    }

    void HttpService::closeConnection_(HttpConnection & http_con) {
        {
            opened_connections_amount_--;
            std::lock_guard lock(queue_mutex_);
            http_con.close();
            log::info("Closed connection valid " + std::to_string(http_con.isValid()));
            available_connections_.push(&http_con);
        }
    }

    void HttpService::addConnection_(HttpConnection && http_con) {
        connections_.emplace_back(std::move(http_con));
        connections_.back().openEpoll();
    }

    void HttpService::setRunning_(bool b) {
        running_ = b;
    }

    bool HttpService::isRunning() const {
        return running_;
    }

    void HttpService::workerRun_() {
        std::array<::epoll_event, event_queue_size> event_queue{};
        while (isRunning() || opened_connections_amount_) {
            int events_count = client_epoll_.wait(event_queue.data(), event_queue.size(), -1);
            log::info("client event: " + std::to_string(events_count));
            for (int i = 0; i < events_count; i++) {
                if (event_queue[i].data.ptr == nullptr) {
                    throw HttpException("Invalid event ptr");
                }
                HttpConnection & http_con = *reinterpret_cast<HttpConnection *>(event_queue[i].data.ptr);
                if (http_con.isValid()) {
                    std::lock_guard lock(http_con.getMutex());
                    log::debug(std::to_string(http_con.getDescriptor().getFd()) + " "
                            + std::to_string((long long)event_queue[i].data.ptr));

                    if (http_con.getDescriptor().isValid()) {
                        log::debug("OK");
                    } else {
                        log::debug("WARN");
                    }

                    if (event_queue[i].events & EPOLLOUT) {
                        log::info("OUT");
                        http_con.writeUntilEagain();
                        if (!http_con.isWriting()) {
                            http_con.refreshTime();
                            http_con.unsubscribe(net::WRITE);
                        } else {
                            http_con.resubscribe();
                        }
                    }
                    if (event_queue[i].events & EPOLLIN) {
                        log::info("IN");
                        http_con.readUntilEagain();
                        bool keep_alive = false;
                        if (listener_) {
                            if (http_con.requestAvailable()) {
                                HttpRequest &request = http_con.getRequest();
                                if (request.find("Connection") != request.end()
                                    && request["Connection"] == "keep-alive") {
                                    keep_alive = true;
                                }
                                log::info("keep-alive " + std::to_string(keep_alive));
                                http_con.refreshTime();
                                listener_->onRequest(http_con);
                                if (!keep_alive && !http_con.isWriting()) {
                                    closeConnection_(http_con);
                                }
                            }
                        }
                        if (keep_alive || http_con.isWriting()) {
                            http_con.resubscribe();
                        }
                    }
                }
            }
        }
    }

    int HttpService::watchdogDowntimeDuration_() {
        return std::chrono::duration_cast<ms>(steady_clock::now() - last_watchdog_run_).count();
    }

    void HttpService::watchdog_() {
        log::info("watchdog");
        for (auto it = connections_.begin(); it != connections_.end();) {
            {
                HttpConnection &http_con = *it;
                std::lock_guard lock(http_con.getMutex());
                log::info("(watchdog) is valid: " + std::to_string(http_con.isValid()));
                log::info("(watchdog) write ongoing: " + std::to_string(http_con.isWriting()));
                if (http_con.isValid() && (http_con.downtimeDuration() > max_downtime)) {
                    it++;
                    closeConnection_(http_con);
                } else {
                    it++;
                }
            }
        }
        last_watchdog_run_ = steady_clock::now();
    }

}
