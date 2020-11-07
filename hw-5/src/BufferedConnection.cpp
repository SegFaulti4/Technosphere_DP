#include "BufferedConnection.h"
#include "NetException.h"

namespace net {

    BufferedConnection::BufferedConnection(tcp::Connection con, EPoll & epoll) :
            epoll_(epoll), connection_(std::move(con)) {
        epoll_.ctl_(EPOLL_CTL_ADD, connection_.get_descriptor().get_fd(), EPOLLRDHUP);
    }

    BufferedConnection& BufferedConnection::operator=(BufferedConnection && other) noexcept {
        if (this != &other) {
            connection_ = std::move(other.connection_);
            read_buf_ = std::move(other.read_buf_);
            write_buf_ = std::move(other.write_buf_);
            epoll_ = std::move(other.epoll_);
        }
        return *this;
    }

    void BufferedConnection::subscribe(Event_subscribe event) {
        if (subscription_ ^ event) {
            subscription_ |= event;
            epoll_.ctl_(EPOLL_CTL_MOD, connection_.get_descriptor().get_fd(), subscription_);
        }
    }

    void BufferedConnection::unsubscribe(Event_subscribe event) {
        if (event & subscription_) {
            subscription_ ^= event;
            epoll_.mod(connection_.get_descriptor().get_fd(),
                       static_cast<Event_subscribe>(subscription_));
        }
    }

    const tcp::Descriptor & BufferedConnection::get_descriptor() {
        return connection_.get_descriptor();
    }

    void BufferedConnection::close() {
        connection_.close();
    }

    void BufferedConnection::buf_read() {
        size_t res = connection_.read(tmp_, sizeof(tmp_));
        if (!res) {
            throw NetException("Nothing was read");
        }
        read_buf_.resize(read_buf_.size() + res);
        read_buf_.insert(read_buf_.begin() + read_buf_.size(), tmp_, &tmp_[res - 1]);
    }

    void BufferedConnection::buf_write() {
        size_t res = connection_.write(tmp_, sizeof(tmp_));
        if (!res) {
            throw NetException("Nothing was written");
        }
        read_buf_.resize(read_buf_.size() + res);
        read_buf_.insert(read_buf_.begin() + read_buf_.size(), tmp_, &tmp_[res - 1]);
    }

    std::string & BufferedConnection::get_read_buf() {
        return read_buf_;
    }

    std::string & BufferedConnection::get_write_buf() {
        return write_buf_;
    }

}
