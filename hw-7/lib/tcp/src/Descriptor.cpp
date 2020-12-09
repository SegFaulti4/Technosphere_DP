#include "Descriptor.h"
#include <string>
#include <utility>
#include <iostream>
#include "unistd.h"
#include "TcpException.h"

namespace tcp {

    Descriptor::Descriptor(int fd) {
        fd_ = fd;
    }

    Descriptor::Descriptor(Descriptor &&other) noexcept {
        *this = std::move(other);
    }

    Descriptor::~Descriptor() {
        try {
            close();
        } catch(std::exception &) {}
    }

    void Descriptor::close() {
        if (isValid()) {
            if (::close(fd_) == -1) {
                fd_ = -1;
                throw TcpException("Close error");
            }
            fd_ = -1;
        }
    }

    void Descriptor::setFd(int fd) {
        close();
        fd_ = fd;
    }

    int Descriptor::getFd() const {
        return fd_;
    }

    size_t Descriptor::read(void *data, size_t count) const {
        if (!isValid()) {
            throw TcpException("Read from invalid descriptor");
        }
        ssize_t res = ::read(fd_, data, count);
        if (res == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "er" << std::endl;
                throw TcpException("Read error");
            } else {
                throw TcpBlockException("Read would block");
            }
        }
        return res;
    }

    size_t Descriptor::write(const void *data, size_t count) const {
        if (!isValid()) {
            throw TcpException("Write to invalid descriptor");
        }
        ssize_t res = ::write(fd_, data, count);
        if (res == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                throw TcpException("Write error");
            } else {
                throw TcpBlockException("Write would block");
            }
        }
        return res;
    }

    void Descriptor::writeExact(const void *data, size_t len) const {
        size_t cur = 0;
        size_t res;
        while (cur < len) {
            res = write(static_cast<const char *>(data) + cur, len - cur);
            if (res == 0) {
                throw TcpException("Write exact failed");
            }
            cur += res;
        }
    }

    void Descriptor::readExact(void *data, size_t len) const {
        size_t cur = 0;
        size_t res;
        while (cur < len) {
            if (!(res = read(static_cast<char *>(data) + cur, len - cur))) {
                throw TcpException("Read exact failed");
            }
            cur += res;
        }
    }

    Descriptor &Descriptor::operator=(Descriptor &&other) noexcept {
        if (this != &other) {
            fd_ = std::exchange(other.fd_, -1);
        }
        return *this;
    }

    bool Descriptor::isValid() const {
        return fd_ != -1;
    }

}
