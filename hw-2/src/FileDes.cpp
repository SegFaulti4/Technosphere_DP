#include "FileDes.h"
#include <stdexcept>
#include "unistd.h"
#include "fcntl.h"

namespace log {

    FileDes::FileDes(int fd) {
        fd_ = fd;
    }

    FileDes::FileDes(const std::string &path, int flags, mode_t mode = 0) {
        if (mode == 0) {
            fd_ = ::open(path.data(), flags);
        } else {
            fd_ = ::open(path.data(), flags, mode);
        }
        if (fd_ == -1) {
            throw std::runtime_error("Open fd error");
        }
    }

    FileDes::~FileDes() {
        FileDes::close();
    }

    void FileDes::close() {
        if (fd_ != -1) {
            if (::close(fd_) == -1) {
                throw std::runtime_error("Close fd error");
            }
        }
        fd_ = -1;
    }

    void FileDes::open(const std::string &path, int flags, mode_t mode) {
        FileDes::close();
        if (mode == 0) {
            fd_ = ::open(path.data(), flags);
        } else {
            fd_ = ::open(path.data(), flags, mode);
        }
        if (fd_ == -1) {
            throw std::runtime_error("Open fd error");
        }
    }

    size_t FileDes::read(void *buf, size_t count) {
        return ::read(fd_, buf, count);
    }

    size_t FileDes::write(const void *buf, size_t count) {
        return ::write(fd_, buf, count);
    }

    void FileDes::replace(int fd = -1) {
        FileDes::close();
        fd_ = fd;
    }

    void FileDes::flush() {
        if (fsync(fd_) == -1) {
            throw std::runtime_error("Fsync error");
        }
    }

}
