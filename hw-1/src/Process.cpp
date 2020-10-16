#include "../include/Process.h"
#include "../include/Pipe.h"
#include <stdexcept>
#include <sys/wait.h>
#include "unistd.h"
#include "fcntl.h"

Process::Process(const std::string& path) {
    pid_ = -1;
    int rd_pipe[2];
    int wr_pipe[2];
    if (pipe(rd_pipe) == -1) {
        throw std::runtime_error("Pipe creation error");
    }
    if (pipe(wr_pipe) == -1) {
        throw std::runtime_error("Pipe creation error");
    }
    if (::close(rd_pipe[1]) == -1) {
        throw std::runtime_error("Close fd error");
    }
    if (::close(wr_pipe[0]) == -1) {
        throw std::runtime_error("Close fd error");
    }
    rd_.replace(rd_pipe[0]);
    wr_.replace(wr_pipe[1]);
    Pipe tmp_pipe(O_CLOEXEC);
    pid_ = fork();
    if (pid_ != -1) {
        if (pid_ == 0) {
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
        if (::read(tmp_pipe.rd(), &tmp, sizeof(tmp)) == -1) {
            throw std::runtime_error("Read error");
        }
    }
}

Process::~Process() {
    if (pid_ != -1) {
        Process::close();
    }
}

void Process::pid_update() {
    int status;
    waitpid(pid_, &status, WNOHANG);
    if (WIFEXITED(status) || WIFEXITED(status)) {
        pid_ = -1;
    }
}

void Process::closeStdin() {
    if (pid_ != -1) {
        rd_.close();
    }
}

void Process::closeStdout() {
    if (pid_ != -1) {
        wr_.close();
    }
}

void Process::close() {
    if (pid_ != -1) {
        kill(pid_, SIGINT);
        int status;
        waitpid(pid_, &status, 0);
        pid_ = -1;
        Process::closeStdin();
        Process::closeStdout();
    }
}

size_t Process::write(const void *data, size_t len) {
    if (data == nullptr) {
        return 0;
    }
    if (pid_ != -1) {
        int res = wr_.write(data, len);
        if (res == -1) {
            throw std::runtime_error("Write error");
        }
        return res;
    }
    return 0;
}

size_t Process::read(void *data, size_t len) {
    if (data == nullptr) {
        return 0;
    }
    if (pid_ != -1) {
        int res = rd_.read(data, len);
        if (res == -1) {
            throw std::runtime_error("Read error");
        }
        return res;
    }
    return 0;
}

void Process::writeExact(const void *data, size_t len) {
    if (data == nullptr) {
        return;
    }
    size_t cur = 0;
    size_t res;
    if (pid_ != -1) {
        while (cur < len) {
            res = Process::write(static_cast<const char*>(data) + cur, len - cur);
            cur += res;
        }
    }
}

void Process::readExact(void *data, size_t len) {
    if (data == nullptr) {
        return;
    }
    size_t cur = 0;
    size_t res;
    if (pid_ != -1) {
        while (cur < len) {
            res = Process::read(static_cast<char*>(data) + cur, len - cur);
            cur += res;
        }
    }
}
