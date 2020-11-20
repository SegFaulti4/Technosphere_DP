#include "HttpConnection.h"

namespace http {

    HttpConnection::HttpConnection(tcp::Connection con, EPoll & epoll) :
            epoll_(epoll), connection_(std::move(con)) {
        epoll_.ctl_(EPOLL_CTL_ADD, connection_.get_descriptor().get_fd(), EPOLLRDHUP);
    }

    HttpConnection::~HttpConnection() {
        try {
            epoll_.del(connection_.get_descriptor().get_fd());
        } catch (std::exception &) {}
    }

    HttpConnection& HttpConnection::operator=(HttpConnection && other) noexcept {
        if (this != &other) {
            connection_ = std::move(other.connection_);
            read_buf_ = std::move(other.read_buf_);
            write_buf_ = std::move(other.write_buf_);
            epoll_ = std::move(other.epoll_);
        }
        return *this;
    }

    void HttpConnection::subscribe(Event_subscribe event) {
        if (subscription_ ^ event) {
            subscription_ |= event;
            epoll_.ctl_(EPOLL_CTL_MOD, connection_.get_descriptor().get_fd(), subscription_);
        }
    }

    void HttpConnection::resubscribe() {
        epoll_.ctl_(EPOLL_CTL_MOD, connection_.get_descriptor().get_fd(), subscription_);
    }

    const tcp::Descriptor & HttpConnection::get_descriptor() {
        return connection_.get_descriptor();
    }

    void HttpConnection::buf_read() {
        size_t res = connection_.read(tmp_, sizeof(tmp_));
        if (!res) {
            throw HttpException("Nothing was read");
        }
        read_buf_.append(tmp_, res);
    }

    void HttpConnection::buf_write() {
        size_t res = connection_.write(write_buf_.data(), write_buf_.size());
        if (!res) {
            throw HttpException("Nothing was written");
        }
        write_buf_.erase(0, res);
    }

    void HttpConnection::read_until_eagain() {
        while (true) {
            int pos = read_buf_.size();
            try {
                buf_read();
            } catch (tcp::TcpBlockException &) {
                break;
            } catch (HttpException &) {
            } catch (std::exception &exc) {
                throw exc;
            }
            if (read_buf_.find("\r\n", pos) != std::string::npos) {
                // парсим
            }
        }
    }

    void HttpConnection::write_until_eagain() {
        while (write_buf_.empty()) {
            try {
                buf_write();
            } catch (tcp::TcpBlockException &) {
                break;
            } catch (HttpException &) {
            } catch (std::exception &exc) {
                throw exc;
            }
        }
    }

    std::string & HttpConnection::get_read_buf() {
        return read_buf_;
    }

    bool HttpConnection::request_available() {
        return !request_queue_.empty();
    }

    bool HttpConnection::write_ongoing() {
        return !write_buf_.empty();
    }

    void HttpConnection::refresh_time() {
        last_used_ = steady_clock::now();
    }

    int HttpConnection::downtime_duration() {
        return std::chrono::duration_cast<ms>(steady_clock::now() - last_used_).count();
    }

}
