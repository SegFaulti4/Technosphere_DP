#include "HttpConnection.h"
#include "HttpException.h"
#include "log.h"

namespace http {

    const std::string ALLOWED_REQUEST_METHODS[] = {
            "GET"
    };

    const std::string ALLOWED_TYPE_VERSIONS[] = {
            "HTTP/1.1",
            "HTTP/1.0"
    };

    const std::string ALLOWED_MESSAGE_HEADERS[] = {
            "Connection: ",
            "Content-Length: ",
            "User-Agent",
            "Accept",
            "Host"
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
        setEpollData(this);
    }

    HttpConnection & HttpConnection::operator=(HttpConnection && other) noexcept {
        if (this != &other) {
            last_used_ = other.last_used_;
            request_ = std::move(other.request_);
            request_available_ = other.request_available_;
            setEpollData(this);
            reinterpret_cast<net::BufferedConnection *>(this)->operator=(std::move(other));
        }
        return *this;
    }

    void HttpConnection::parseRequestLine() {
        int pos = 0;
        std::string & str = read_buf_;
        int end_of_req_line = str.find("\r\n", pos);
        log::info("end req line found");
        std::string tmp;
        for (const std::string &method : ALLOWED_REQUEST_METHODS) {
            if (str.find(method, pos) == pos) {
                tmp = method;
                break;
            }
        }
        if (tmp.empty()) {
            throw HttpException("Wrong request format");
        }
        log::info("method " + tmp);
        request_["method"] = tmp;
        pos += tmp.size();
        int tmp_pos = str.find(' ', pos + 1);
        if (str.find(' ', pos) != pos || tmp_pos == std::string::npos
            || tmp_pos > end_of_req_line || tmp_pos == pos + 1) {
            throw HttpException("Wrong request format");
        }
        log::info("URI " + str.substr(pos + 1, tmp_pos - pos));
        request_["URI"] = str.substr(pos + 1, tmp_pos - pos);
        pos = tmp_pos + 1;
        tmp.clear();
        for (const std::string &type_version : ALLOWED_TYPE_VERSIONS) {
            if (str.find(type_version, pos) == pos) {
                tmp = type_version;
                break;
            }
        }
        if (tmp.empty() || (pos + tmp.size()) != end_of_req_line
            || (tmp_pos = str.find('/', pos)) == std::string::npos) {
            throw HttpException("Wrong request format");
        }
        log::info("type " + str.substr(pos, tmp_pos - pos));
        log::info("version " +
                  str.substr(tmp_pos + 1, end_of_req_line - tmp_pos - 1));
        request_["type"] = str.substr(pos, tmp_pos - pos);
        request_["version"] = str.substr(tmp_pos + 1, end_of_req_line - tmp_pos - 1);
        str.erase(0, end_of_req_line + 2);
    }

    void HttpConnection::parseMessageHeaders() {
        int pos;
        std::string & str = read_buf_;
        while ((pos = str.find("\r\n")) != std::string::npos) {
            if (str.find("\r\n") == 0) {
                str.erase(0, 2);
                if (request_.find("Content-Length") == request_.end()) {
                    if (!str.empty()) {
                        throw HttpException("Only one request is allowed at a time");
                    }
                    setRequestAvailable();
                }
                break;
            }
            std::string tmp;
            for (const std::string &message_header : ALLOWED_MESSAGE_HEADERS) {
                if (str.find(message_header)) {
                    tmp = message_header;
                }
            }
            if (tmp.empty()) {
                throw HttpException("Wrong request format");
            }
            tmp.erase(tmp.size() - 2);
            if (request_.find(tmp) != request_.end()) {
                throw HttpException("Wrong request format");
            }
            request_[tmp] = str.substr(tmp.size() + 2, pos);
            str.erase(0, pos + 2);
        }
    }

    void HttpConnection::parseBody() {
        std::string & str = read_buf_;
        int pos = str.find("\r\n\r\n");
        if (pos != std::string::npos) {
            if (str.size() - pos - 4 > 0) {
                throw HttpException("Only one request is allowed at a time");
            }
            if (str.size() - 4 != std::stoi(request_["Content-Length"])) {
                throw HttpException("Wrong content length");
            }
            request_["body"] = str.substr(0, pos);
            setRequestAvailable();
            str.clear();
        } else if (str.size() > std::stoi(request_["Content-Length"])) {
            throw HttpException("Message body size exceeded");
        }
    }

    void HttpConnection::parseRequest() {
        std::string & str = read_buf_;
        if (request_.find("Content-Length") != request_.end()) {
            parseBody();
        } else if (str.find("\r\n\r\n") != std::string::npos) {
            parseRequestLine();
            parseMessageHeaders();
            if (!request_available_) {
                parseBody();
            }
        } else if (str.size() > MAX_REQUEST_LENGTH) {
            throw HttpException("Request size exceeded");
        }
    }

    void HttpConnection::readUntilEagain() {
        if (request_available_ || isWriting()) {
            throw HttpException("Only one request is allowed at a time");
        }
        while (true) {
            try {
                readIntoBuf();
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
        event_ = 0;
    }

    void HttpConnection::setEpollData(void * ptr) {
        reinterpret_cast<net::BufferedConnection *>(this)->setEpollData(ptr);
    }

    void HttpConnection::openEpoll() {
        reinterpret_cast<net::BufferedConnection *>(this)->openEpoll();
        log::info(std::to_string(connection_.getDescriptor().getFd()) + " " + std::to_string((long long) this)
                + " " + std::to_string((long long) epoll_data_) + " " + std::to_string(subscription_));
    }

    void HttpConnection::unsubscribe(net::EventSubscribe event) {
        reinterpret_cast<net::BufferedConnection *>(this)->unsubscribe(event);
    }

    std::mutex & HttpConnection::getMutex() {
        return mutex_;
    }

    HttpRequest && HttpConnection::getRequest() {
        return std::move(request_);
    }


    void HttpConnection::writeResponse(const std::string &response) {
        write_buf_.append(response);
        subscribe(net::EventSubscribe::WRITE);
    }

    void HttpConnection::setEvent(int event) {
        event_ = event;
    }

    int HttpConnection::getEvent() {
        return event_;
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
}
