#include <iostream>
#include "Service.h"
#include "NetException.h"

namespace net {

    constexpr size_t MAX_READ_LENGTH = 4096;

    BufferedConnection::BufferedConnection(tcp::Connection && con, EPoll & epoll) :
            epoll_(epoll), connection_(std::move(con)){
        setEpollData(this);
    }

    BufferedConnection::BufferedConnection(BufferedConnection && other) noexcept  : epoll_(other.epoll_),
            connection_(std::move(other.connection_)) {
        read_buf_ = std::move(other.read_buf_);
        write_buf_ = std::move(other.write_buf_);
        epoll_data_ = std::exchange(other.epoll_data_, nullptr);
        subscription_ = std::exchange(other.subscription_, 0);
    }

    void BufferedConnection::openEpoll() {
        epoll_.add(connection_.getDescriptor(), epoll_data_, subscription_);
    }

    void BufferedConnection::close() {
        connection_.close();
        subscription_ = 0;
        read_buf_.clear();
        write_buf_.clear();
    }

    void BufferedConnection::subscribe(EventSubscribe event) {
        if (subscription_ ^ static_cast<int>(event)) {
            subscription_ |= static_cast<int>(event);
            epoll_.mod(connection_.getDescriptor(), epoll_data_, subscription_);
        }
    }

    void BufferedConnection::unsubscribe(EventSubscribe event) {
        if (static_cast<int>(event) & subscription_) {
            subscription_ ^= static_cast<int>(event);
            epoll_.mod(connection_.getDescriptor(), epoll_data_, subscription_);
        }
    }

    const tcp::Descriptor & BufferedConnection::getDescriptor() const {
        return connection_.getDescriptor();
    }

    void BufferedConnection::readIntoBuf() {
        read_buf_.resize(read_buf_.size() + MAX_READ_LENGTH);
        size_t res;
        try {
            res = connection_.read(read_buf_.data() + read_buf_.size() - MAX_READ_LENGTH, MAX_READ_LENGTH);
        } catch (tcp::TcpBlockException &) {
            read_buf_.resize(read_buf_.size() - MAX_READ_LENGTH);
            throw NetBlockException("Read would block");
        }
        read_buf_.resize(read_buf_.size() - MAX_READ_LENGTH + res);
        if (!res) {
            throw NetException("Nothing was read");
        }
    }

    void BufferedConnection::writeFromBuf() {
        size_t res;
        try {
            res = connection_.write(write_buf_.data(), write_buf_.size());
        } catch (tcp::TcpBlockException &) {
            throw NetBlockException("Write would block");
        }
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
