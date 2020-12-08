#include "Service.h"
#include "NetException.h"

namespace net {

    constexpr size_t max_read_length_ = 4096;

    BufferedConnection::BufferedConnection(tcp::Connection con, EPoll & epoll) :
            epoll_(epoll), connection_(std::move(con)){
        epoll_data_ = this;
    }

    BufferedConnection::BufferedConnection(BufferedConnection && other) noexcept  : epoll_(other.epoll_),
            connection_(std::move(other.connection_)) {
        read_buf_ = std::move(other.read_buf_);
        write_buf_ = std::move(other.write_buf_);
        epoll_data_ = other.epoll_data_;
        subscription_ = other.subscription_;
    }

    void BufferedConnection::openEpoll() {
        epoll_data_ = this;
        epoll_.ctl(EPOLL_CTL_ADD, connection_.getDescriptor().getFd(), epoll_data_, subscription_);
    }

    void BufferedConnection::close() {
        connection_.close();
        subscription_ = 0;
        read_buf_.clear();
        write_buf_.clear();
    }

    void BufferedConnection::subscribe(EventSubscribe event) {
        if (subscription_ ^ event) {
            subscription_ |= event;
            epoll_.ctl(EPOLL_CTL_MOD, connection_.getDescriptor().getFd(), epoll_data_, subscription_);
        }
    }

    void BufferedConnection::unsubscribe(EventSubscribe event) {
        if (event & subscription_) {
            subscription_ ^= event;
            epoll_.ctl(EPOLL_CTL_MOD, connection_.getDescriptor().getFd(), epoll_data_, subscription_);
        }
    }

    const tcp::Descriptor & BufferedConnection::getDescriptor() const {
        return connection_.getDescriptor();
    }

    void BufferedConnection::readIntoBuf() {
        read_buf_.resize(read_buf_.size() + max_read_length_);
        size_t res = connection_.read(read_buf_.data() + read_buf_.size() - max_read_length_, max_read_length_);
        read_buf_.resize(read_buf_.size() - max_read_length_ + res);
        if (!res) {
            throw NetException("Nothing was read");
        }
    }

    void BufferedConnection::writeFromBuf() {
        size_t res = connection_.write(write_buf_.data(), write_buf_.size());
        if (!res) {
            throw NetException("Nothing was written");
        }
        write_buf_.erase(0, res);
    }

    std::string & BufferedConnection::getReadBuf() {
        return read_buf_;
    }

    std::string & BufferedConnection::getWriteBuf() {
        return write_buf_;
    }

    void BufferedConnection::ctl(int op, int event) {
        epoll_.ctl(op, connection_.getDescriptor().getFd(), epoll_data_, event);
    }

    void BufferedConnection::setEpollData(void *ptr) {
        epoll_data_ = ptr;
    }

    BufferedConnection & BufferedConnection::operator=(BufferedConnection && other) noexcept {
        if (connection_.getDescriptor().getFd() != other.connection_.getDescriptor().getFd()) {
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
