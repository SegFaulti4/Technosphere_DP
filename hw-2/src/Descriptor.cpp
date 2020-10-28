#include "Descriptor.h"

namespace log {

    Descriptor::Descriptor() = default;

    Descriptor::Descriptor(int fd) {
        fd_ = fd;
    }

    Descriptor::Descriptor(Descriptor && other) noexcept  {
        *this = std::move(other);
    }

    Descriptor::~Descriptor() {
        ::close(fd_);
    }

    void Descriptor::close() {
        if (fd_ != -1) {
            if (::close(fd_) == -1) {
                fd_ = -1;
                throw std::runtime_error("Close error\n");
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
        if (fd_ == -1) {
            throw std::runtime_error("Read from invalid descriptor\n");
        }
        ssize_t res = ::read(fd_, data, count);
        if (res == -1) {
            throw std::runtime_error("Read error\n");
        }
        return res;
    }

    size_t Descriptor::write(const void *data, size_t count) const {
        if (fd_ == -1) {
            throw std::runtime_error("Write to invalid descriptor\n");
        }
        ssize_t res = ::write(fd_, data, count);
        if (res == -1) {
            throw std::runtime_error("Write error\n");
        }
        return res;
    }

    void Descriptor::writeExact(const void *data, size_t len) const {
        size_t cur = 0;
        size_t res;
        while (cur < len) {
            if (!(res = write(static_cast<const char*>(data) + cur, len - cur))) {
                break;
            }
            cur += res;
        }
    }

    void Descriptor::readExact(void *data, size_t len) const {
        size_t cur = 0;
        size_t res;
        while (cur < len) {
            if (!(res = read(static_cast<char*>(data) + cur, len - cur))) {
                break;
            }
            cur += res;
        }
    }

    Descriptor & Descriptor::operator=(Descriptor &&other) noexcept {
        if (this != &other) {
            close();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

}
