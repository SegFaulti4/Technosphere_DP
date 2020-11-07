#include "Descriptor.h"

namespace tcp {

    Descriptor::Descriptor() = default;

    Descriptor::Descriptor(int fd) {
        fd_ = fd;
    }

    Descriptor::Descriptor(Descriptor &&other) noexcept {
        *this = std::move(other);
    }

    Descriptor::~Descriptor() {
        ::close(fd_);
    }

    void Descriptor::close() {
        if (is_valid()) {
            if (::close(fd_) == -1) {
                fd_ = -1;
                throw TcpException("Close error");
            }
            fd_ = -1;
        }
    }

    void Descriptor::set_fd(int fd) {
        close();
        fd_ = fd;
    }

    int Descriptor::get_fd() const {
        return fd_;
    }

    size_t Descriptor::read(void *data, size_t count) const {
        if (!is_valid()) {
            throw TcpException("Read from invalid descriptor");
        }
        ssize_t res = ::read(fd_, data, count);
        if (res == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                throw TcpException("Read error");
            } else {
                throw TcpTimeoutException("Read would block");
            }
        }
        return res;
    }

    size_t Descriptor::write(const void *data, size_t count) const {
        if (!is_valid()) {
            throw TcpException("Write to invalid descriptor");
        }
        ssize_t res = ::write(fd_, data, count);
        if (res == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                throw TcpException("Write error");
            } else {
                throw TcpTimeoutException("Write would block");
            }
        }
        return res;
    }

    void Descriptor::writeExact(const void *data, size_t len) const {
        size_t cur = 0;
        size_t res;
        while (cur < len) {
            if (!(res = write(static_cast<const char *>(data) + cur, len - cur))) {
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
            close();
            fd_ = std::exchange(other.fd_, -1);
        }
        return *this;
    }

    bool Descriptor::is_valid() const {
        return fd_ != -1;
    }

}
