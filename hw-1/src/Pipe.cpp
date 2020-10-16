#include "../include/Pipe.h"
#include <stdexcept>
#include "unistd.h"

Pipe::Pipe(int flag) {
    fd_[0] = -1;
    fd_[1] = -1;
    if (::pipe2(fd_, flag) == -1) {
        throw std::runtime_error("Pipe creation error");
    }
}

Pipe::~Pipe() {
    Pipe::close();
}

int Pipe::rd() {
    return fd_[0];
}

int Pipe::wr() {
    return fd_[1];
}

void Pipe::close_rd() {
    if (fd_[0] != -1) {
        if (::close(fd_[0]) == -1) {
            throw std::runtime_error("Close fd error");
        }
        fd_[0] = -1;
    }
}

void Pipe::close_wr() {
    if (fd_[1] != -1) {
        if (::close(fd_[1]) == -1) {
            throw std::runtime_error("Close fd error");
        }
        fd_[1] = -1;
    }
}

void Pipe::close() {
    Pipe::close_rd();
    Pipe::close_wr();
}
