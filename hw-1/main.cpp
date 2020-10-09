#include "unistd.h"
#include "fcntl.h"
#include <iostream>
#include <sys/wait.h>

class Pipe {
private:
    int fd[2] = {-1, -1};
public:
    explicit Pipe(int flag = 0) {
        if (::pipe2(fd, flag) == -1) {
            throw std::runtime_error("Pipe creation error");
        }
    }

    ~Pipe() {
        ::close(fd[0]);
        ::close(fd[1]);
    }

    int rd() {
        return fd[0];
    }

    int wr() {
        return fd[1];
    }

    int close_rd() {
        int res = ::close(fd[0]);
        fd[0] = -1;
        return res;
    }

    int close_wr() {
        int res = ::close(fd[1]);
        fd[1] = -1;
        return res;
    }

    std::pair<int, int> close() {
        return std::make_pair(close_rd(), close_wr());
    }
};

class Process {
private:
    Pipe rd_sub, wr_sub;
    pid_t pid = -1;

public:
    explicit Process(const std::string& path) {
        pid = fork();
        if (pid != -1) {
            Pipe tmp_pipe(O_CLOEXEC);
            if (pid == 0) {
                execlp(path.data(), "child", nullptr);
                exit(-1);
            }
            char tmp;
            if (::read(tmp_pipe.rd(), &tmp, sizeof(tmp)) == -1) {
                throw std::runtime_error("Read error");
            }
        }
    }

    ~Process() {
        pid_update();
        if (!pid) {
            closeStdin();
            closeStdout();
        } else if (pid != -1) {
            close();
        }
    }

    void pid_update() {
        int status;
        waitpid(pid, &status, WNOHANG);
        if (WIFEXITED(status) || WIFEXITED(status)) {
            pid = -1;
        }
    }

    void closeStdin() {
        pid_update();
        if (!pid || pid != -1) {
            rd_sub.close();
        }
    }

    void closeStdout() {
        pid_update();
        if (!pid || pid != -1) {
            wr_sub.close();
        }
    }

    void close() {
        pid_update();
        if (pid && pid != -1) {
            kill(pid, SIGINT);
            int status;
            waitpid(pid, &status, 0);
            pid = -1;
        }
    }

    size_t write(const void *data, size_t len) {
        if (data == nullptr) {
            return 0;
        }
        pid_update();
        if (!pid) {
            return ::write(rd_sub.wr(), data, len);
        } else if (pid != -1) {
            return ::write(wr_sub.rd(), data, len);
        }
        return 0;
    }

    size_t read(void *data, size_t len) {
        if (data == nullptr) {
            return 0;
        }
        pid_update();
        if (!pid) {
            return ::read(wr_sub.rd(), data, len);
        } else if (pid != -1) {
            return ::read(rd_sub.rd(), data, len);
        }
        return 0;
    }

    void writeExact(const void *data, size_t len) {
        if (data == nullptr) {
            return;
        }
        pid_update();
        size_t cur = 0;
        size_t res;
        if (!pid) {
            while (cur < len) {
                res = ::write(rd_sub.wr(),
                               (const unsigned char *)data + cur, len - cur);
                if (res <= 0) {
                    return;
                }
                cur += res;
            }
        } else if (pid != -1) {
            while (cur < len) {
                res = ::write(wr_sub.wr(),
                               (const unsigned char *)data + cur, len - cur);
                if (res <= 0) {
                    return;
                }
                cur += res;
            }
        }
    }

    void readExact(void *data, size_t len) {
        if (data == nullptr) {
            return;
        }
        pid_update();
        size_t cur = 0;
        size_t res;
        if (!pid) {
            while (cur < len) {
                res = ::read(wr_sub.rd(),
                               (unsigned char *)data + cur, len - cur);
                if (res <= 0) {
                    return;
                }
                cur += res;
            }
        } else if (pid != -1) {
            while (cur < len) {
                res = ::read(rd_sub.rd(),
                               (unsigned char *)data + cur, len - cur);
                if (res <= 0) {
                    return;
                }
                cur += res;
            }
        }
    }
};

int main() {
    Process cur("./main.cpp");
    return 0;
}
