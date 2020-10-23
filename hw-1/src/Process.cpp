#include "Process.h"
#include "Pipe.h"

Process::Process(const std::string& path) {
    pid_ = -1;
    Pipe rd_pipe(0);
    Pipe wr_pipe(0);
    Pipe tmp_pipe(O_CLOEXEC);
    rd_.set_fd(dup(rd_pipe.rd()));
    wr_.set_fd(dup(wr_pipe.wr()));
    pid_ = fork();
    if (pid_ == -1) {
        rd_pipe.close();
        wr_pipe.close();
        tmp_pipe.close();
        rd_.close();
        wr_.close();
        throw std::runtime_error("Fork error");
    }
    if (pid_ == 0) {
        ::close(rd_pipe.wr());
        ::close(wr_pipe.rd());
        dup2(rd_pipe.wr(), STDOUT_FILENO);
        dup2(wr_pipe.rd(), STDIN_FILENO);
        rd_.close();
        wr_.close();
        int pos = path.rfind('/');
        if (pos >= path.length()) {
            pos = 0;
        }
        std::string name = path.substr(pos);
        execlp(path.data(), name.data(), nullptr);
        exit(-1);
    }
    char tmp;
    tmp_pipe.close_wr();
    if (::read(tmp_pipe.rd(), &tmp, sizeof(tmp)) == -1) {
        throw std::runtime_error("Read error");
    }
    int status;
    waitpid(pid_, &status, WNOHANG);
}

Process::~Process() {
    close();
}

void Process::pid_update() {
    int status;
    waitpid(pid_, &status, WNOHANG);
    if (WIFEXITED(status) || WIFEXITED(status)) {
        pid_ = -1;
    }
}

void Process::closeStdin() {
    rd_.close();
}

void Process::closeStdout() {
    wr_.close();
}

void Process::close() {
    if (pid_ != -1) {
        kill(pid_, SIGINT);
        int status;
        waitpid(pid_, &status, 0);
        pid_ = -1;
        closeStdin();
        closeStdout();
    }
}

ssize_t Process::write(const void *data, size_t len) {
    ssize_t res = wr_.write(data, len);
    if (res == -1) {
        throw std::runtime_error("Write error");
    }
    return res;
}

ssize_t Process::read(void *data, size_t len) {
    ssize_t res = rd_.read(data, len);
    if (res == -1) {
        throw std::runtime_error("Read error");
    }
    return res;
}

void Process::writeExact(const void *data, size_t len) {
    ssize_t cur = 0;
    ssize_t res;
    while (cur < len) {
        if (!(res = write(static_cast<const char*>(data) + cur, len - cur))) {
            break;
        }
        cur += res;
    }
}

void Process::readExact(void *data, size_t len) {
    ssize_t cur = 0;
    ssize_t res;
    while (cur < len) {
        if (!(res = read(static_cast<char*>(data) + cur, len - cur))) {
            break;
        }
        cur += res;
    }
}
