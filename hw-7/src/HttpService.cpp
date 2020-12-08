#include "HttpService.h"
#include "HttpException.h"
#include <thread>

namespace http {

    constexpr size_t EVENT_QUEUE_SIZE = 1024;
    constexpr int MAX_DOWNTIME = 2000;
    constexpr int MAX_CONNECTION_AMOUNT = 10000;
    constexpr int WATCHDOG_PERIOD = 10;

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
        setRunning(false);
        server_.close();
    }

    void HttpService::run() {
        if (!isRunning()) {
            std::vector<std::thread> threads;
            threads.reserve(worker_amount_);
            for (int i = 0; i < worker_amount_; i++) {
                threads.emplace_back(&http::HttpService::workerRun, this);
            }
            setRunning(true);
            server_.setTimeout(WATCHDOG_PERIOD);
            while (isRunning()) {
                if (opened_connections_amount_ < MAX_CONNECTION_AMOUNT) {
                    addConnection();
                } else {
                    log::warn("Max connection amount reached");
                    std::this_thread::sleep_for(std::chrono::milliseconds(WATCHDOG_PERIOD));
                }
                if (watchdogDowntimeDuration() > WATCHDOG_PERIOD) {
                    watchdog();
                }
            }
            for (std::thread& t : threads) {
                t.join();
            }
        }
    }

    void HttpService::closeConnection(HttpConnection & http_con) {
        std::lock_guard lock(queue_mutex_);
        opened_connections_amount_--;
        http_con.close();
        log::info("Closed connection valid " + std::to_string(http_con.isValid()));
        available_connections_.push(&http_con);
    }

    void HttpService::addConnection() {
        try {
            HttpConnection http_con(server_.accept(), client_epoll_);
            log::info("New connection");
            opened_connections_amount_++;
            std::lock_guard lock(queue_mutex_);
            if (available_connections_.empty()) {
                log::info("No connections available");
                connections_.emplace_back(std::move(http_con));
                pushTimingConnection(connections_.back());
                connections_.back().setEpollData(&connections_.back());
                connections_.back().setRoutine(coroutine::create(&HttpService::onEvent, this, &connections_.back()));
                connections_.back().openEpoll();
            } else {
                auto con = available_connections_.front();
                *con = std::move(http_con);
                pushTimingConnection(*con);
                con->setEpollData(con);
                available_connections_.pop();
                con->setRoutine(coroutine::create(&HttpService::onEvent, this, con));
                con->openEpoll();
            }
        } catch (tcp::TcpBlockException &) {
            log::debug("Accept would block");
        } catch (std::exception &exc) {
            throw exc;
        }
    }

    void HttpService::pushTimingConnection(HttpConnection & http_con) {
        timing_connections_.push_back(&http_con);
        http_con.refreshTime();
    }

    void HttpService::setRunning(bool b) {
        running_ = b;
    }

    bool HttpService::isRunning() const {
        return running_;
    }

    void HttpService::workerRun() {
        std::array<::epoll_event, EVENT_QUEUE_SIZE> event_queue{};
        while (isRunning() || opened_connections_amount_) {
            int events_count = client_epoll_.wait(event_queue.data(), event_queue.size(), -1);
            log::info("client event: " + std::to_string(events_count));
            for (int i = 0; i < events_count; i++) {
                if (event_queue[i].data.ptr == nullptr) {
                    throw HttpException("Invalid event ptr");
                }
                HttpConnection & http_con = *reinterpret_cast<HttpConnection *>(event_queue[i].data.ptr);
                std::lock_guard lock(http_con.getMutex());

                log::debug(std::to_string(http_con.getDescriptor().getFd()) + " "
                           + std::to_string((long long)event_queue[i].data.ptr));

                http_con.setEvent(event_queue[i].events);
                http_con.resume();
            }
        }
    }

    int HttpService::watchdogDowntimeDuration() {
        return std::chrono::duration_cast<ms>(steady_clock::now() - last_watchdog_run_).count();
    }

    void HttpService::watchdog() {
        for (auto it = timing_connections_.begin(); it != timing_connections_.end();) {
            HttpConnection & http_con = **it;
            std::lock_guard lock(http_con.getMutex());
            if (http_con.refreshIsDelayed()) {
                auto prev_it = it;
                it++;
                timing_connections_.erase(prev_it);
                pushTimingConnection(http_con);
            } else if (http_con.downtimeDuration() > MAX_DOWNTIME) {
                auto prev_it = it;
                it++;
                timing_connections_.erase(prev_it);
                if (http_con.isValid()) {
                    closeConnection(http_con);
                }
            } else {
                break;
            }
        }
        last_watchdog_run_ = steady_clock::now();
    }

    void HttpService::onEvent(HttpConnection *http_con) {
        while (http_con->isValid()) {
            if (http_con->getEvent() & EPOLLOUT) {
                log::info("OUT");
                http_con->writeUntilEagain();
                if (!http_con->isWriting()) {
                    http_con->setDelayedRefresh();
                    http_con->unsubscribe(net::EventSubscribe::WRITE);
                } else {
                    http_con->resubscribe();
                }
            }
            if (http_con->getEvent() & EPOLLIN) {
                log::info("IN");
                http_con->readUntilEagain();
                bool keep_alive = false;
                if (listener_) {
                    if (http_con->requestAvailable()) {
                        HttpRequest &request = http_con->getRequest();
                        if (request.find("Connection") != request.end()
                            && request["Connection"] == "keep-alive") {
                            keep_alive = true;
                        }
                        log::info("keep-alive " + std::to_string(keep_alive));
                        http_con->setDelayedRefresh();
                        listener_->onRequest(*http_con);
                        if (!keep_alive && !http_con->isWriting()) {
                            closeConnection(*http_con);
                        }
                    }
                }
                if (keep_alive || http_con->isWriting()) {
                    http_con->resubscribe();
                }
            }
            http_con->setEvent(0);
            coroutine::yield();
        }
    }

}
