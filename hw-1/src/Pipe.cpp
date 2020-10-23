#include "Pipe.h"

Pipe::Pipe(int flag) {
    int fd[2];
    if (::pipe2(fd, flag) == -1) {
        throw std::runtime_error("Pipe creation error");
    }
    rd_.set_fd(fd[0]);
    wr_.set_fd(fd[1]);
}

int Pipe::rd() {
    return rd_.get_fd();
}

int Pipe::wr() {
    return wr_.get_fd();
}

void Pipe::close_rd() {
    rd_.close();
}

void Pipe::close_wr() {
    wr_.close();
}

void Pipe::close() {
    close_rd();
    close_wr();
}
