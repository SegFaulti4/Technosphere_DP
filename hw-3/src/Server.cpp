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

    void Server::listen(const std::string & addr, unsigned port,
                   int max_connection) {
        close();
        dscrptr_.set_fd(::socket(AF_INET, SOCK_STREAM, 0));
        if (dscrptr_.get_fd() == -1) {
            throw std::runtime_error("Socket init error\n");
        }
        int opt = 1;
        if (::setsockopt(dscrptr_.get_fd(), SOL_SOCKET,
                         SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            throw std::runtime_error("Set socket options error\n");
        }
        addr_in_.sin_family = AF_INET;
        addr_in_.sin_port = ::htons(port);
        if (::inet_aton(addr.data(), &addr_in_.sin_addr) == 0) {
            throw std::runtime_error("Incorrect ip\n");
        }
        if (bind(dscrptr_.get_fd(), reinterpret_cast<sockaddr*>(&addr_in_),
                 sizeof(addr_in_)) == -1) {
            throw std::runtime_error("Bind error\n");
        }
        if (::listen(dscrptr_.get_fd(), max_connection) == -1) {
            throw std::runtime_error("Listen error\n");
        }
    }

    void Server::listen(unsigned addr, unsigned port,
                        int max_connection) {
        close();
        dscrptr_.set_fd(::socket(AF_INET, SOCK_STREAM, 0));
        if (dscrptr_.get_fd() == -1) {
            throw std::runtime_error("Socket init error\n");
        }
        int opt = 1;
        if (::setsockopt(dscrptr_.get_fd(), SOL_SOCKET,
                         SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            throw std::runtime_error("Set socket options error\n");
        }
        addr_in_.sin_family = AF_INET;
        addr_in_.sin_port = ::htons(port);
        addr_in_.sin_addr = { ::htonl(addr) };
        if (bind(dscrptr_.get_fd(), reinterpret_cast<sockaddr*>(&addr_in_),
                 sizeof(addr_in_)) == -1) {
            throw std::runtime_error("Bind error\n");
        }
        if (::listen(dscrptr_.get_fd(), max_connection) == -1) {
            throw std::runtime_error("Listen error\n");
        }
    }

    Connection Server::accept() {
        sockaddr_in client_addr = {};
        socklen_t addr_size = sizeof(client_addr);
        int acc_socket = ::accept(dscrptr_.get_fd(),
                                  reinterpret_cast<sockaddr*>(&client_addr),
                                  &addr_size);
        if (acc_socket == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return Connection();
            }
            throw std::runtime_error("Accept error\n");
        }
        return Connection(acc_socket);
    }

    void Server::close() {
        dscrptr_.close();
    }

    void Server::set_max_connection(int new_max) {
        close();
        dscrptr_.set_fd(::socket(AF_INET, SOCK_STREAM, 0));
        if (dscrptr_.get_fd() == -1) {
            throw std::runtime_error("Socket init error\n");
        }
        int opt = 1;
        if (::setsockopt(dscrptr_.get_fd(), SOL_SOCKET,
                         SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            throw std::runtime_error("Set socket options error\n");
        }
        if (bind(dscrptr_.get_fd(), reinterpret_cast<sockaddr*>(&addr_in_),
                 sizeof(addr_in_)) == -1) {
            throw std::runtime_error("Bind error\n");
        }
        if (::listen(dscrptr_.get_fd(), new_max) == -1) {
            throw std::runtime_error("Listen error\n");
        }
    }

    void Server::set_timeout_(ssize_t ms, int opt) {
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

    void Server::set_timeout(ssize_t ms) {
        set_timeout_(ms, SO_SNDTIMEO);
        set_timeout_(ms, SO_RCVTIMEO);
    }

    Server & Server::operator=(Server &&other) noexcept {
        dscrptr_ = std::move(other.dscrptr_);
        return *this;
    }

}
