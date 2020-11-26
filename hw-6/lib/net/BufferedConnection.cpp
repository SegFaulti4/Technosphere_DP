#include "Service.h"
#include "NetException.h"

namespace net {

    BufferedConnection::BufferedConnection(tcp::Connection con, EPoll & epoll) :
            epoll_(epoll), connection_(std::move(con)){
        epoll_data_.type = CONNECTION;
        epoll_data_.fd = connection_.get_descriptor().get_fd();
        epoll_data_.ptr = this;
        epoll_data_.meta = 0;
        epoll_.ctl(EPOLL_CTL_ADD, &epoll_data_, subscription_);
    }

    BufferedConnection::BufferedConnection(BufferedConnection && other) noexcept  : epoll_(other.epoll_),
            connection_(std::move(other.connection_)) {
        read_buf_ = std::move(other.read_buf_);
        write_buf_ = std::move(other.write_buf_);
        epoll_data_ = other.epoll_data_;
        subscription_ = other.subscription_;
    }

    void BufferedConnection::subscribe(Event_subscribe event) {
        if (subscription_ ^ event) {
            subscription_ |= event;
            epoll_.ctl(EPOLL_CTL_MOD, &epoll_data_, subscription_);
        }
    }

    void BufferedConnection::unsubscribe(Event_subscribe event) {
        if (event & subscription_) {
            subscription_ ^= event;
            epoll_.ctl(EPOLL_CTL_MOD, &epoll_data_, subscription_);
        }
    }

    const tcp::Descriptor & BufferedConnection::get_descriptor() const {
        return connection_.get_descriptor();
    }

    void BufferedConnection::read_into_buf() {
        read_buf_.resize(read_buf_.size() + max_read_length_);
        size_t res = connection_.read(read_buf_.data() + read_buf_.size() - max_read_length_, max_read_length_);
        read_buf_.resize(read_buf_.size() - max_read_length_ + res);
        if (!res) {
            throw NetException("Nothing was read");
        }
    }

    void BufferedConnection::write_into_buf() {
        size_t res = connection_.write(write_buf_.data(), write_buf_.size());
        if (!res) {
            throw NetException("Nothing was written");
        }
        write_buf_.erase(0, res);
    }

    std::string & BufferedConnection::get_read_buf() {
        return read_buf_;
    }

    std::string & BufferedConnection::get_write_buf() {
        return write_buf_;
    }

    void BufferedConnection::ctl(int op, int event) {
        epoll_.ctl(op, &epoll_data_, event);
    }

    void BufferedConnection::set_meta(int meta) {
        epoll_data_.meta = meta;
    }

    void BufferedConnection::set_ptr(void *ptr) {
        epoll_data_.ptr = ptr;
    }

    BufferedConnection & BufferedConnection::operator=(BufferedConnection && other) noexcept {
        if (connection_.get_descriptor().get_fd() != other.connection_.get_descriptor().get_fd()) {
            connection_ = std::move(other.connection_);
            read_buf_ = std::move(other.read_buf_);
            write_buf_ = std::move(other.write_buf_);
            epoll_ = std::move(other.epoll_);
            epoll_data_ = other.epoll_data_;
            subscription_ = other.subscription_;
        }
        return *this;
    }

}
