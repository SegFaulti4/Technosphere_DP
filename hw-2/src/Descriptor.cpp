#include "Descriptor.h"

namespace log {

    Descriptor::Descriptor(int fd) {
        fd_ = fd;
    }

    Descriptor::Descriptor(Descriptor && other) noexcept  {
        fd_ = -1;
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

    ssize_t Descriptor::read(void *data, size_t count) const {
        if (fd_ == -1) {
            throw std::runtime_error("Read from invalid descriptor\n");
        }
        return ::read(fd_, data, count);
    }

    ssize_t Descriptor::write(const void *data, size_t count) const {
        if (fd_ == -1) {
            throw std::runtime_error("Write to invalid descriptor\n");
        }
        return ::write(fd_, data, count);
    }

    void Descriptor::writeExact(const void *data, size_t len) const {
        ssize_t cur = 0;
        ssize_t res;
        while (cur < len) {
            if (!(res = write(static_cast<const char*>(data) + cur, len - cur))) {
                break;
            }
            cur += res;
        }
    }

    void Descriptor::readExact(void *data, size_t len) const {
        ssize_t cur = 0;
        ssize_t res;
        while (cur < len) {
            if (!(res = read(static_cast<char*>(data) + cur, len - cur))) {
                break;
            }
            cur += res;
        }
    }

    Descriptor & Descriptor::operator=(Descriptor &&other) noexcept {
        if (this != &other) {
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

}
