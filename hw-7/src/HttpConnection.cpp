#include "HttpConnection.h"
#include "HttpException.h"
#include "log.h"
#include <unordered_set>
#include <sstream>

namespace http {

    const std::unordered_set<std::string> ALLOWED_MESSAGE_HEADERS = {
            "Connection",
            "Content-Length",
            "User-Agent",
            "Accept",
            "Host",
    };

    constexpr int MAX_REQUEST_LENGTH = 4096;

    HttpConnection::HttpConnection(tcp::Connection && con, net::EPoll &epoll)
            : net::BufferedConnection(std::move(con), epoll) {
        subscription_ = EPOLLET | EPOLLONESHOT | EPOLLIN | EPOLLRDHUP;
    }

    HttpConnection::HttpConnection(HttpConnection && other) noexcept : net::BufferedConnection(std::move(other)) {
        request_ = std::move(other.request_);
        other.request_.clear();
        request_available_ = std::exchange(other.request_available_, false);
        last_used_ = other.last_used_;
        keep_alive_ = std::exchange(other.keep_alive_, false);
        setEpollData(this);
    }

    HttpConnection & HttpConnection::operator=(HttpConnection && other) noexcept {
        if (this != &other) {
            last_used_ = other.last_used_;
            request_ = std::move(other.request_);
            request_available_ = other.request_available_;
            keep_alive_ = std::exchange(other.keep_alive_, false);
            setEpollData(this);
            reinterpret_cast<net::BufferedConnection *>(this)->operator=(std::move(other));
        }
        return *this;
    }

    void HttpConnection::parseRequestLine(std::istringstream &in) {
        std::string tmp;
        std::getline(in, tmp, '\r');
        request_["request line"] = tmp;
        log::debug(request_["request line"]);
        in.ignore();
    }

    void HttpConnection::parseMessageHeaders(std::istringstream &in) {
        std::string header;
        std::string tmp;
        std::getline(in, header, ':');
        log::debug("header: " + header);
        while (!header.empty()) {
            if (ALLOWED_MESSAGE_HEADERS.find(header) == ALLOWED_MESSAGE_HEADERS.end()) {
                throw HttpException("Wrong request format");
            }
            in.ignore(1);
            std::getline(in, tmp, '\r');
            request_[header] = tmp;
            log::debug(request_[header]);
            in.ignore(1);
            header.clear();
            std::getline(in, header, ':');
            log::debug("header: " + header);
        }
    }

    void HttpConnection::parseHead() {
        int pos = read_buf_.find("\r\n\r\n");
        std::string head = read_buf_.substr(0, pos);
        read_buf_.erase(0, pos + 4);
        std::istringstream in(head);

        parseRequestLine(in);
        parseMessageHeaders(in);

        if (request_.find("Content-Length") == request_.end()) {
            request_available_ = true;
            read_buf_.clear();
        }
    }

    void HttpConnection::parseBody() {
        request_["body"] = read_buf_.substr(0, read_buf_.find("\r\n\r\n"));
        read_buf_.clear();
        request_available_ = true;
    }

    void HttpConnection::parseRequest() {
        while (read_buf_.find("\r\n\r\n") != std::string::npos && !requestAvailable()) {
            if (request_.find("Content-Length") != request_.end()) {
                parseBody();
            } else {
                parseHead();
            }
        }
        if (request_available_) {
            if (request_.find("Connection") != request_.end()) {
                log::info("Connection: |" + request_["Connection"] + "|");
            }
            if (request_.find("Connection") != request_.end() &&
                    (request_["Connection"] == "Keep-Alive" || request_["Connection"] == "keep-alive")) {
                keep_alive_ = true;
            } else {
                keep_alive_ = false;
            }
        }
    }

    void HttpConnection::readUntilEagain() {
        if (request_available_ || isWriting()) {
            throw HttpException("Only one request is allowed at a time");
        }
        while (true) {
            try {
                readIntoBuf();
                if (read_buf_.size() > MAX_REQUEST_LENGTH) {
                    throw HttpException("Request size exceeded");
                }
            } catch (net::NetBlockException &) {
                break;
            } catch (net::NetException &) {
                throw HttpException("Nothing was read");
            }
        }
        parseRequest();
    }

    void HttpConnection::writeUntilEagain() {
        while (isWriting()) {
            try {
                writeFromBuf();
            } catch (net::NetBlockException &) {
                break;
            } catch (net::NetException &) {
                throw HttpException("Nothing was written");
            }
        }
    }

    bool HttpConnection::requestAvailable() const {
        return request_available_;
    }

    int HttpConnection::downtimeDuration() {
        return std::chrono::duration_cast<ms>(steady_clock::now() - last_used_).count();
    }

    void HttpConnection::refreshTime() {
        watchdog_refresh_flag_ = false;
        last_used_ = steady_clock::now();
    }

    void HttpConnection::resubscribe() {
        epoll_.mod(connection_.getDescriptor(), epoll_data_, subscription_);
    }

    bool HttpConnection::isWriting() {
        return !write_buf_.empty();
    }

    bool HttpConnection::isValid() {
        return connection_.getDescriptor().isValid();
    }

    bool HttpConnection::refreshIsDelayed() {
        return watchdog_refresh_flag_;
    }

    void HttpConnection::setDelayedRefresh() {
        watchdog_refresh_flag_ = true;
    }

    const tcp::Descriptor & HttpConnection::getDescriptor() const {
        return reinterpret_cast<const net::BufferedConnection *>(this)->getDescriptor();
    }

    void HttpConnection::close() {
        connection_.close();
        subscription_ = 0;
        request_.clear();
        request_available_ = false;
        routine_ = 0;
        keep_alive_ = false;
        watchdog_refresh_flag_ = false;
    }

    void HttpConnection::setEpollData(void * ptr) {
        reinterpret_cast<net::BufferedConnection *>(this)->setEpollData(ptr);
    }

    void HttpConnection::openEpoll() {
        reinterpret_cast<net::BufferedConnection *>(this)->openEpoll();
        log::info(std::to_string(connection_.getDescriptor().getFd()) + " " + std::to_string((long long) this)
                + " " + std::to_string((long long) epoll_data_) + " " + std::to_string(subscription_));
    }

    void HttpConnection::subscribe(net::EventSubscribe event) {
        reinterpret_cast<net::BufferedConnection *>(this)->subscribe(event);
    }

    void HttpConnection::unsubscribe(net::EventSubscribe event) {
        reinterpret_cast<net::BufferedConnection *>(this)->unsubscribe(event);
    }

    std::mutex & HttpConnection::getMutex() {
        return mutex_;
    }

    HttpRequest && HttpConnection::getRequest() {
        request_available_ = false;
        return std::move(request_);
    }


    void HttpConnection::writeResponse(const std::string &response) {
        write_buf_.append(response);
        subscribe(net::EventSubscribe::WRITE);
    }

    void HttpConnection::setRoutine(coroutine::routine_t routine) {
        routine_ = routine;
    }

    void HttpConnection::resume() {
        coroutine::resume(routine_);
    }

    bool HttpConnection::keepAlive() const {
        return keep_alive_;
    }

    void HttpConnection::setRequestAvailable() {
        if (request_.find("Connection") != request_.end() &&
                (request_["Connection"] == "keep-alive" || request_["Connection"] == "Keep-alive")) {
            keep_alive_ = true;
        } else {
            keep_alive_ = false;
        }
        request_available_ = true;
    }

    void HttpConnection::switchEvent() {
        epoll_.mod(connection_.getDescriptor(), epoll_data_, subscription_ - EPOLLOUT + EPOLLIN);
    }
}
