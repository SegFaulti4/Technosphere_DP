#include "Server.h"

namespace tcp {

    Server::Server() = default;

    Server::Server(const std::string & addr, unsigned port,
                   int max_connection) {
        listen(addr, port, max_connection);
    }

    Server::Server(unsigned addr, unsigned port, int max_connection) {
        listen(addr, port, max_connection);
    }

    Server::Server(Server && other) noexcept {
        *this = std::move(other);
    }

    void Server::listen_(unsigned addr, unsigned port, int max_connection) {
        Descriptor fd(::socket(AF_INET, SOCK_STREAM, 0));
        if (!fd.isValid()) {
            throw TcpException("Socket init error");
        }
        int opt = 1;
        if (::setsockopt(fd.getFd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            throw TcpException("Set socket options error");
        }
        sockaddr_in addr_in{};
        addr_in.sin_family = AF_INET;
        addr_in.sin_port = ::htons(port);
        addr_in.sin_addr = { addr };
        if (::bind(fd.getFd(), reinterpret_cast<sockaddr*>(&addr_in), sizeof(addr_in)) == -1) {
            throw TcpException("Bind error");
        }
        if (::listen(fd.getFd(), max_connection) == -1) {
            throw TcpException("Listen error");
        }

        descriptor_ = std::move(fd);
    }

    void Server::listen(const std::string & addr, unsigned port, int max_connection) {
        sockaddr_in addr_in{};
        if (::inet_aton(addr.data(), &addr_in.sin_addr) == 0) {
            throw TcpException("Incorrect ip");
        }
        listen_(addr_in.sin_addr.s_addr, port, max_connection);
    }

    void Server::listen(unsigned addr, unsigned port, int max_connection) {
        listen_(::htonl(addr), port, max_connection);
    }

    Connection Server::accept() {
        sockaddr_in client_addr = {};
        socklen_t addr_size = sizeof(client_addr);

        int acc_socket = ::accept4(descriptor_.getFd(),reinterpret_cast<sockaddr*>(&client_addr),
                                  &addr_size, SOCK_NONBLOCK);
        if (acc_socket == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                throw TcpBlockException("Accept would block");
            }
            throw TcpException("Accept error");
        }
        return Connection(acc_socket);
    }

    void Server::close() {
        descriptor_.close();
    }

    void Server::setMaxConnection(int new_max) {
        if (descriptor_.isValid()) {
            if (::listen(descriptor_.getFd(), new_max) == -1) {
                throw TcpException("Listen error");
            }
        }
    }

    void Server::setTimeout_(ssize_t ms, int opt) {
        timeval timeout{ .tv_sec = ms / 1000, .tv_usec = ms % 1000};
        if (setsockopt(descriptor_.getFd(), SOL_SOCKET, opt,
                       &timeout, sizeof(timeout)) == -1) {
            throw TcpException("Socket option set error");
        }
    }

    void Server::setTimeout(ssize_t ms) {
        setTimeout_(ms, SO_SNDTIMEO);
        setTimeout_(ms, SO_RCVTIMEO);
    }

    Server & Server::operator=(Server &&other) noexcept {
        descriptor_ = std::move(other.descriptor_);
        return *this;
    }

    const Descriptor & Server::getDescriptor() {
        return descriptor_;
    }

}
