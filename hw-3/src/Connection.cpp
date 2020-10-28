#include "Connection.h"

namespace tcp {

    Connection::Connection() = default;

    Connection::Connection(const std::string & addr, unsigned port) {
        connect(addr, port);
    }

    Connection::Connection(unsigned addr, unsigned port) {
        connect(addr, port);
    }

    Connection::Connection(Connection && other) noexcept {
        *this = std::move(other);
    }

    Connection::Connection(int socket) {
        dscrptr_.set_fd(socket);
    }

    Connection::~Connection() {
        ::close(dscrptr_.get_fd());
    }

    void Connection::connect(const std::string & addr, unsigned port) {
        dscrptr_.close();
        dscrptr_.set_fd(socket(AF_INET, SOCK_STREAM, 0));
        if (dscrptr_.get_fd() == -1) {
            throw std::runtime_error("Socket init error\n");
        }
        sockaddr_in addr_in{};
        addr_in.sin_family = AF_INET;
        addr_in.sin_port = ::htons(port);
        if (::inet_aton(addr.data(), &addr_in.sin_addr) == 0) {
            throw std::runtime_error("Incorrect port\n");
        }
        if (::connect(dscrptr_.get_fd(), reinterpret_cast <sockaddr*>(&addr_in), sizeof(addr_in))) {
            throw std::runtime_error("Connect error\n");
        }
    }

    void Connection::connect(unsigned addr, unsigned port) {
        dscrptr_.close();
        dscrptr_.set_fd(socket(AF_INET, SOCK_STREAM, 0));
        if (dscrptr_.get_fd() == -1) {
            throw std::runtime_error("Socket init error\n");
        }
        sockaddr_in addr_in{};
        addr_in.sin_family = AF_INET;
        addr_in.sin_port = ::htons(port);
        addr_in.sin_addr = { ::htonl(addr) };
        if (::connect(dscrptr_.get_fd(), reinterpret_cast <sockaddr*>(&addr_in), sizeof(addr_in))) {
            throw std::runtime_error("Connect error\n");
        }
    }

    void Connection::close() {
        dscrptr_.close();
    }

    size_t Connection::read(void *buf, size_t count) {
        return dscrptr_.read(buf, count);
    }

    size_t Connection::write(const void *buf, size_t count) {
        return dscrptr_.write(buf, count);
    }

    void Connection::readExact(void *buf, size_t count) {
        dscrptr_.readExact(buf, count);
    }

    void Connection::writeExact(const void *buf, size_t count) {
        dscrptr_.writeExact(buf, count);
    }

    void Connection::set_timeout_(ssize_t ms, int opt) {
        timeval timeout{ .tv_sec = ms / 1000, .tv_usec = ms % 1000};
        if (setsockopt(dscrptr_.get_fd(), SOL_SOCKET, opt,
                       &timeout, sizeof(timeout)) == -1) {
            throw std::runtime_error("Socket option set error\n");
        }
        timeval get_timeout = {};
        socklen_t tmp = sizeof(timeout);
        if (getsockopt(dscrptr_.get_fd(), SOL_SOCKET, opt, &get_timeout, &tmp) == -1) {
            throw std::runtime_error("Socket option get error\n");
        }
        if (timeout.tv_usec != get_timeout.tv_usec || timeout.tv_sec != get_timeout.tv_sec) {
            throw std::runtime_error("Failed to set timeout\n");
        }
    }

    void Connection::set_timeout(ssize_t ms) {
        set_timeout_(ms, SO_SNDTIMEO);
        set_timeout_(ms, SO_RCVTIMEO);
    }

    Connection & Connection::operator=(Connection && other) noexcept {
        this->dscrptr_ = std::move(other.dscrptr_);
        return *this;
    }

}
