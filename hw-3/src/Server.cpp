#include "Server.h"
#include <iostream>

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
        if (!fd.is_valid()) {
            throw TcpException("Socket init error");
        }
        int opt = 1;
        if (::setsockopt(fd.get_fd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            throw TcpException("Set socket options error");
        }
        sockaddr_in addr_in{};
        addr_in.sin_family = AF_INET;
        addr_in.sin_port = ::htons(port);
        addr_in.sin_addr = { addr };
        if (::bind(fd.get_fd(), reinterpret_cast<sockaddr*>(&addr_in), sizeof(addr_in)) == -1) {
            throw TcpException("Bind error");
        }
        if (::listen(fd.get_fd(), max_connection) == -1) {
            throw TcpException("Listen error");
        }

        dscrptr_ = std::move(fd);
        addr_in_ = addr_in;
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

        int acc_socket = ::accept(dscrptr_.get_fd(), reinterpret_cast<sockaddr*>(&client_addr), &addr_size);
        if (acc_socket == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                throw TcpTimeoutException("Accept would block");
            }
            throw TcpException("Accept error");
        }
        return Connection(acc_socket);
    }

    void Server::close() {
        dscrptr_.close();
    }

    void Server::set_max_connection(int new_max) {
        if (dscrptr_.is_valid()) {
            if (::listen(dscrptr_.get_fd(), new_max) == -1) {
                throw TcpException("Listen error");
            }
        }
    }

    void Server::set_timeout_(ssize_t ms, int opt) {
        timeval timeout{ .tv_sec = ms / 1000, .tv_usec = ms % 1000};
        if (::setsockopt(dscrptr_.get_fd(), SOL_SOCKET, opt,
                       &timeout, sizeof(timeout)) == -1) {
            throw TcpException("Socket option set error");
        }
    }

    void Server::set_timeout(ssize_t ms) {
        set_timeout_(ms, SO_SNDTIMEO);
        set_timeout_(ms, SO_RCVTIMEO);
    }

    Server & Server::operator=(Server &&other) noexcept {
        dscrptr_ = std::move(other.dscrptr_);
        addr_in_ = other.addr_in_;
        return *this;
    }

}
