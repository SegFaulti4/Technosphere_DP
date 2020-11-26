#include "HttpConnection.h"

namespace http {

    HttpConnection::HttpConnection(net::BufferedConnection && buf_con, int mutex_idx) :
            connection_(std::move(buf_con)) {
        mutex_idx_ = mutex_idx;
        connection_.set_meta(mutex_idx);
        connection_.set_ptr(this);
        connection_.ctl(EPOLL_CTL_MOD, subscription_);
    }

    HttpConnection::HttpConnection(HttpConnection && other)  noexcept : connection_(std::move(other.connection_)) {
        request_ = std::move(other.request_);
        request_available_ = other.request_available_;
        last_used_ = other.last_used_;
        subscription_ = other.subscription_;
        mutex_idx_ = other.mutex_idx_;
    }

    void HttpConnection::read_until_eagain() {
        if (request_available_) {
            throw HttpException("Only one request is allowed at a time");
        }
        while (true) {
            try {
                connection_.read_into_buf();
            } catch (tcp::TcpBlockException &) {
                break;
            } catch (net::NetException &) {
            } catch (std::exception & exc) {
                throw exc;
            }
            std::string & str = connection_.get_read_buf();
            switch (mode_) {

                case REQUEST_LINE: {
                    int pos = 0;
                    int end_of_req_line = str.find("\r\n", pos);
                    if (end_of_req_line != std::string::npos) {
                        std::string tmp;
                        for (const std::string &method : AllowedRequestMethods) {
                            if (str.find(method, pos) == pos) {
                                tmp = method;
                                break;
                            }
                        }
                        if (tmp.empty()) {
                            throw HttpException("Wrong request format");
                        }
                        request_["method"] = tmp;
                        pos += tmp.size();
                        int tmp_pos = str.find(' ', pos + 1);
                        if (str.find(' ', pos) != pos || tmp_pos == std::string::npos
                            || tmp_pos > end_of_req_line || tmp_pos == pos + 1) {
                            throw HttpException("Wrong request format");
                        }
                        request_["URI"] = str.substr(pos + 1, tmp_pos - pos + 1);
                        pos = tmp_pos + 1;
                        tmp.clear();
                        for (const std::string &type_version : AllowedType_Versions) {
                            if (str.find(type_version, pos) == pos) {
                                tmp = type_version;
                                break;
                            }
                        }
                        if (tmp.empty() || (pos + tmp.size()) != end_of_req_line
                            || (tmp_pos = str.find('/', pos)) == std::string::npos) {
                            throw HttpException("Wrong request format");
                        }
                        request_["type"] = str.substr(pos, tmp_pos - pos);
                        request_["version"] = str.substr(tmp_pos + 1, end_of_req_line - tmp_pos - 1);
                        str.erase(0, end_of_req_line + 2);
                        mode_ = MESSAGE_HEADERS;
                    } else {
                        if (str.size() > max_request_line_length) {
                            throw HttpException("Request line size exceeded");
                        }
                        break;
                    }
                }

                case MESSAGE_HEADERS: {
                    int pos;
                    while ((pos = str.find("\r\n")) != std::string::npos) {
                        if (str.find("\r\n\r\n") == 0) {
                            str.erase(0, 4);
                            if (request_.find("Content-Length") != request_.end()) {
                                mode_ = MESSAGE_BODY;
                            } else {
                                if (!str.empty()) {
                                    throw HttpException("Only one request is allowed at a time");
                                }
                                request_available_ = true;
                                mode_ = REQUEST_LINE;
                            }
                            break;
                        }
                        std::string tmp;
                        for (const std::string &message_header : AllowedMessageHeaders) {
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
                    if (mode_ != MESSAGE_BODY) {
                        if (str.size() > max_message_headers_length) {
                            throw HttpException("Message header size exceeded");
                        }
                        break;
                    }
                }

                case MESSAGE_BODY: {
                    int pos = str.find("\r\n\r\n");
                    if (pos != std::string::npos) {
                        if (str.size() - pos - 4 > 0) {
                            throw HttpException("Only one request is allowed at a time");
                        }
                        request_["body"] = str.substr(0, pos);
                        request_available_ = true;
                        str.erase(0, pos + 4);
                        mode_ = REQUEST_LINE;
                        break;
                    } else if (str.size() > max_message_body_length) {
                        throw HttpException("Message body size exceeded");
                    }
                    break;
                }

            }
        }
        refresh_time();
    }

    void HttpConnection::write_until_eagain() {
        while (connection_.get_write_buf().empty()) {
            try {
                connection_.write_into_buf();
            } catch (tcp::TcpBlockException &) {
                break;
            } catch (net::NetException &) {
            } catch (std::exception & exc) {
                throw exc;
            }
        }
        refresh_time();
    }

    bool HttpConnection::request_available() const {
        return request_available_;
    }

    void HttpConnection::refresh_time() {
        last_used_ = steady_clock::now();
    }

    int HttpConnection::downtime_duration() {
        return std::chrono::duration_cast<ms>(steady_clock::now() - last_used_).count();
    }

    void HttpConnection::resubscribe() {
        connection_.ctl(EPOLL_CTL_MOD, subscription_);
        refresh_time();
    }

    void HttpConnection::subscribe(net::Event_subscribe event) {
        if (subscription_ ^ event) {
            subscription_ |= event;
            connection_.ctl(EPOLL_CTL_MOD, subscription_);
        }
    }

    void HttpConnection::unsubscribe(net::Event_subscribe event) {
        if (event & subscription_) {
            subscription_ ^= event;
            connection_.ctl(EPOLL_CTL_MOD, subscription_);
        }
    }

    const tcp::Descriptor & HttpConnection::getDescriptor() const {
        return connection_.get_descriptor();
    }


    std::string & HttpConnection::get_read_buf() {
        return connection_.get_read_buf();
    }

    bool HttpConnection::write_ongoing() {
        return !connection_.get_write_buf().empty();
    }

    HttpConnection & HttpConnection::operator=(HttpConnection && other)  noexcept {
        if (connection_.get_descriptor().get_fd() != other.connection_.get_descriptor().get_fd()) {
            connection_ = std::move(other.connection_);
            request_ = std::move(other.request_);
            request_available_ = other.request_available_;
            subscription_ = other.subscription_;
            last_used_ = other.last_used_;
            mutex_idx_ = other.mutex_idx_;
        }
        return *this;
    }

    int HttpConnection::get_mutex_idx() const {
        return mutex_idx_;
    }

    void HttpConnection::set_valid(bool b) {
        valid_ = b;
    }

    bool HttpConnection::is_valid() const {
        return valid_;
    }

    HttpRequest & HttpConnection::get_request() {
        return request_;
    }

    void HttpConnection::clear_request() {
        request_.clear();
        request_available_ = false;
    }

}
