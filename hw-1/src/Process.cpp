#include "Process.h"
#include "Pipe.h"

Process::Process(const std::string& path) {
    Pipe rd_pipe;
    Pipe wr_pipe;
    Pipe tmp_pipe(O_CLOEXEC);
    rd_.set_fd(dup(rd_pipe.rd()));
    if (rd_.get_fd() == -1) {
        throw std::runtime_error("Dup error\n");
    }
    wr_.set_fd(dup(wr_pipe.wr()));
    if (wr_.get_fd() == -1) {
        throw std::runtime_error("Dup error\n");
    }
    pid_ = fork();
    if (pid_ == -1) {
        throw std::runtime_error("Fork error\n");
    }
    if (pid_ == 0) {
        rd_pipe.close_rd();
        wr_pipe.close_wr();
        if (::dup2(rd_pipe.wr(), STDOUT_FILENO) == -1) {
            throw std::runtime_error("Dup2 error\n");
        }
        if (::dup2(wr_pipe.rd(), STDIN_FILENO) == -1) {
            throw std::runtime_error("Dup2 error\n");
        }
        rd_pipe.close_rd();
        wr_pipe.close_wr();
        rd_.close();
        wr_.close();
        size_t pos = path.rfind('/');
        if (pos == std::string::npos) {
            pos = 0;
        }
        std::string name = path.substr(pos);
        execlp(path.data(), name.data(), nullptr);
        exit(-1);
    }
    char tmp;
    tmp_pipe.close_wr();
    if (::read(tmp_pipe.rd(), &tmp, sizeof(tmp)) == -1) {
        throw std::runtime_error("Read error\n");
    }
    int status;
    pid_t res = waitpid(pid_, &status, WNOHANG);
    if (res == -1) {
        throw std::runtime_error("Waitpid error\n");
    }
    if (res == pid_) {
        if ((!WIFEXITED(status) && !WIFSIGNALED(status)) || WEXITSTATUS(status)) {
            throw std::runtime_error("Child process ended with error\n");
        }
    }
}

Process::~Process() {
    kill(pid_, SIGINT);
    waitpid(pid_, nullptr, 0);
    ::close(rd_.get_fd());
    ::close(wr_.get_fd());
}

void Process::pid_update() {
    int status;
    pid_t res = waitpid(pid_, &status, WNOHANG);
    if (res == -1) {
        throw std::runtime_error("Waitpid error\n");
    }
    if (res == pid_) {
        if ((WIFEXITED(status) || WIFSIGNALED(status)) && WEXITSTATUS(status) == 0) {
            pid_ = -1;
        } else {
            throw std::runtime_error("Child process ended with error\n");
        }
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

size_t Process::write(const void *data, size_t len) {
    return wr_.write(data, len);
}

size_t Process::read(void *data, size_t len) {
    return rd_.read(data, len);
}

void Process::writeExact(const void *data, size_t len) {
    wr_.writeExact(data, len);
}

void Process::readExact(void *data, size_t len) {
    rd_.readExact(data, len);
}
