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
        descriptor_.set_fd(socket);
    }

    void Connection::connect_(unsigned addr, unsigned port) {
        if (port > USHRT_MAX) {
            throw std::runtime_error("Wrong port number");
        }
        descriptor_.set_fd(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0));
        if (!descriptor_.is_valid()) {
            throw TcpException("Socket init error");
        }
        sockaddr_in addr_in{};
        addr_in.sin_family = AF_INET;
        addr_in.sin_port = ::htons(port);
        addr_in.sin_addr = { addr };
        if (::connect(descriptor_.get_fd(), reinterpret_cast <sockaddr*>(&addr_in), sizeof(addr_in))) {
            throw TcpException("Connect error");
        }
    }

    void Connection::connect(const std::string & addr, unsigned port) {
        sockaddr_in addr_in{};
        if (::inet_aton(addr.data(), &addr_in.sin_addr) == 0) {
            throw TcpException("Incorrect ip");
        }
        connect_(addr_in.sin_addr.s_addr, port);
    }

    void Connection::connect(unsigned addr, unsigned port) {
        connect_(::htonl(addr), port);
    }

    void Connection::close() {
        descriptor_.close();
    }

    size_t Connection::read(void *buf, size_t count) {
        return descriptor_.read(buf, count);
    }

    size_t Connection::write(const void *buf, size_t count) {
        return descriptor_.write(buf, count);
    }

    void Connection::readExact(void *buf, size_t count) {
        descriptor_.readExact(buf, count);
    }

    void Connection::writeExact(const void *buf, size_t count) {
        descriptor_.writeExact(buf, count);
    }

    void Connection::set_timeout_(ssize_t ms, int opt) {
        timeval timeout{ .tv_sec = ms / 1000, .tv_usec = ms % 1000};
        if (setsockopt(descriptor_.get_fd(), SOL_SOCKET, opt,
                       &timeout, sizeof(timeout)) == -1) {
            throw TcpException("Socket option set error");
        }
    }

    void Connection::set_timeout(ssize_t ms) {
        set_timeout_(ms, SO_SNDTIMEO);
        set_timeout_(ms, SO_RCVTIMEO);
    }

    Connection & Connection::operator=(Connection && other) noexcept {
        this->descriptor_ = std::move(other.descriptor_);
        return *this;
    }

    const Descriptor & Connection::get_descriptor() const {
        return descriptor_;
    }

}
