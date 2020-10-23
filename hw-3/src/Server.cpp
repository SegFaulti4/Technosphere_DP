#include "Server.h"

namespace tcp {

    Server::Server() {}

    Server::Server(const std::string addr, unsigned port,
                   unsigned max_connection) {
        listen(addr, port, max_connection);
    }

    Server::Server(Server && other) noexcept {
        *this = std::move(other);
    }

    void Server::listen(const std::string addr, unsigned port,
                   unsigned max_connection) {
        close();
        dscrptr_.set_fd(::socket(AF_INET, SOCK_STREAM, 0));
        if (dscrptr_.get_fd() == -1) {
            throw std::runtime_error("Socket init error\n");
        }
        int opt = 1;
        if (::setsockopt(dscrptr_.get_fd(), SOL_SOCKET,
                         SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            dscrptr_.close();
            throw std::runtime_error("Set socket options error\n");
        }
        addr_in_.sin_family = AF_INET;
        addr_in_.sin_port = htons(port);
        if (::inet_aton(addr.data(), &addr_in_.sin_addr) == 0) {
            dscrptr_.close();
            throw std::runtime_error("Incorrect port\n");
        }
        if (bind(dscrptr_.get_fd(), reinterpret_cast<sockaddr*>(&addr_in_),
                 sizeof(addr_in_)) == -1) {
            dscrptr_.close();
            throw std::runtime_error("Bind error");
        }
        if (::listen(dscrptr_.get_fd(), max_connection) == -1) {
            dscrptr_.close();
            throw std::runtime_error("Listen error");
        }
    }

    Connection Server::accept() {
        sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int acc_socket = ::accept(dscrptr_.get_fd(),
                                  reinterpret_cast<sockaddr*>(&client_addr),
                                  &addr_size);
        if (acc_socket == -1) {
            dscrptr_.close();
            throw std::runtime_error("Accept error\n");
        }
        return Connection(acc_socket);
    }

    void Server::close() {
        dscrptr_.close();
    }

    void Server::set_max_connection(unsigned int new_max) {
        close();
        dscrptr_.set_fd(::socket(AF_INET, SOCK_STREAM, 0));
        if (dscrptr_.get_fd() == -1) {
            throw std::runtime_error("Socket init error\n");
        }
        int opt = 1;
        if (::setsockopt(dscrptr_.get_fd(), SOL_SOCKET,
                         SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            dscrptr_.close();
            throw std::runtime_error("Set socket options error\n");
        }
        if (bind(dscrptr_.get_fd(), reinterpret_cast<sockaddr*>(&addr_in_),
                 sizeof(addr_in_)) == -1) {
            dscrptr_.close();
            throw std::runtime_error("Bind error");
        }
        if (::listen(dscrptr_.get_fd(), new_max) == -1) {
            dscrptr_.close();
            throw std::runtime_error("Listen error");
        }
    }

    void Server::set_timeout(ssize_t ms) {
        timeval timeout{ .tv_sec = ms / 1000, .tv_usec = ms % 1000};
        if (setsockopt(dscrptr_.get_fd(), SOL_SOCKET, SO_SNDTIMEO,
                       &timeout, sizeof(timeout)) == -1) {
            close();
            throw std::runtime_error("Timeout set error\n");
        }
    }

    Server & Server::operator=(Server &&other) noexcept {
        dscrptr_ = std::move(other.dscrptr_);
        return *this;
    }

}
