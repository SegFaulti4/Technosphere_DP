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
            "Content-Length: "
    };

    constexpr int MAX_REQUEST_LINE_LENGTH = 4096;
    constexpr int MAX_MESSAGE_HEADER_LENGTH = 4096;

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

    void HttpConnection::readUntilEagain() {
        if (request_available_ || isWriting()) {
            throw HttpException("Only one request is allowed at a time");
        }
        while (true) {
            try {
                readIntoBuf();
            } catch (tcp::TcpBlockException &) {
                break;
            } catch (net::NetException &) {
                break;
            } catch (std::exception &exc) {
                throw exc;
            }
            std::string &str = read_buf_;
            log::info("mode: " + std::to_string(static_cast<int>(mode_)));
            switch (mode_) {

                case HttpParserMode::REQUEST_LINE: {
                    int pos = 0;
                    int end_of_req_line = str.find("\r\n", pos);
                    if (end_of_req_line != std::string::npos) {
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
                        mode_ = HttpParserMode::MESSAGE_HEADERS;
                    } else {
                        if (str.size() > MAX_REQUEST_LINE_LENGTH) {
                            throw HttpException("Request line size exceeded");
                        }
                        break;
                    }
                }

                case HttpParserMode::MESSAGE_HEADERS: {
                    int pos;
                    while ((pos = str.find("\r\n")) != std::string::npos) {
                        if (str.find("\r\n") == 0) {
                            str.erase(0, 2);
                            if (request_.find("Content-Length") != request_.end()) {
                                mode_ = HttpParserMode::MESSAGE_BODY;
                            } else {
                                if (!str.empty()) {
                                    throw HttpException("Only one request is allowed at a time");
                                }
                                request_available_ = true;
                                mode_ = HttpParserMode::REQUEST_LINE;
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
                    if (mode_ != HttpParserMode::MESSAGE_BODY) {
                        if (str.size() > MAX_MESSAGE_HEADER_LENGTH) {
                            throw HttpException("Message header size exceeded");
                        }
                        break;
                    }
                }

                case HttpParserMode::MESSAGE_BODY: {
                    int pos = str.find("\r\n\r\n");
                    if (pos != std::string::npos) {
                        if (str.size() - pos - 4 > 0) {
                            throw HttpException("Only one request is allowed at a time");
                        }
                        if (str.size() - 4 != std::stoi(request_["Content-Length"])) {
                            throw HttpException("Wrong content length");
                        }
                        request_["body"] = str.substr(0, pos);
                        request_available_ = true;
                        str.erase(0, pos + 4);
                        mode_ = HttpParserMode::REQUEST_LINE;
                        break;
                    } else if (str.size() > std::stoi(request_["Content-Length"])) {
                        throw HttpException("Message body size exceeded");
                    }
                    break;
                }

            }
        }
    }

    void HttpConnection::writeUntilEagain() {
        while (isWriting()) {
            try {
                writeFromBuf();
            } catch (tcp::TcpBlockException &) {
                break;
            } catch (net::NetException &) {
                break;
            } catch (std::exception & exc) {
                throw exc;
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
        mode_ = HttpParserMode::REQUEST_LINE;
        request_available_ = false;
        routine_ = 0;
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

    HttpRequest & HttpConnection::getRequest() {
        return request_;
    }

    void HttpConnection::clearRequest() {
        request_.clear();
        request_available_ = false;
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

}
