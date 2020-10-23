#include "Connection.h"

namespace tcp {

    Connection::Connection() {}

    Connection::Connection(const std::string & addr, unsigned port) {
        connect(addr, port);
    }

    Connection::Connection(Connection && other) noexcept {
        *this = std::move(other);
    }

    Connection::Connection(int socket) {
        dscrptr_.set_fd(socket);
    }

    void Connection::connect(const std::string & addr, unsigned port) {
        dscrptr_.close();
        dscrptr_.set_fd(socket(AF_INET, SOCK_STREAM, 0));
        if (dscrptr_.get_fd() == -1) {
            throw std::runtime_error("Socket init error\n");
        }
        sockaddr_in addr_in{};
        addr_in.sin_family = AF_INET;
        addr_in.sin_port = htons(port);
        if (::inet_aton(addr.data(), &addr_in.sin_addr) == 0) {
            dscrptr_.close();
            throw std::runtime_error("Incorrect port\n");
        }
        if (::connect(dscrptr_.get_fd(), reinterpret_cast <sockaddr*>(&addr_in), sizeof(addr_in))) {
            dscrptr_.close();
            throw std::runtime_error("Connect error\n");
        }
    }

    void Connection::close() {
        dscrptr_.close();
    }

    ssize_t Connection::read(void *buf, size_t count) {
        return dscrptr_.read(buf, count);
    }

    ssize_t Connection::write(const void *buf, size_t count) {
        return dscrptr_.write(buf, count);
    }

    void Connection::readExact(void *buf, size_t count) {
        dscrptr_.readExact(buf, count);
    }

    void Connection::writeExact(const void *buf, size_t count) {
        dscrptr_.writeExact(buf, count);
    }

    void Connection::set_timeout(ssize_t ms) {
        timeval timeout{ .tv_sec = ms / 1000, .tv_usec = ms % 1000};
        if (setsockopt(dscrptr_.get_fd(), SOL_SOCKET, SO_SNDTIMEO,
                       &timeout, sizeof(timeout)) == -1) {
            close();
            throw std::runtime_error("Timeout set error\n");
        }
    }

    Connection & Connection::operator=(Connection && other) noexcept {
        this->dscrptr_ = std::move(other.dscrptr_);
        return *this;
    }

}
