#include "unistd.h"
#include "fcntl.h"
#include <iostream>
#include <sys/wait.h>

class Process {
private:
    int pipe_rd_sub[2];
    int pipe_wr_sub[2];
    pid_t pid;

public :
    explicit Process(const std::string& path) {
        pid = fork();
        if (pid != -1) {
            pipe(pipe_rd_sub);
            pipe(pipe_wr_sub);
            int tmp_pipe[2];
            pipe2(tmp_pipe, O_CLOEXEC);
            if (pid == 0) {
                execlp(path.data(), "child", nullptr);
                exit(-1);
            }
            char tmp;
            ::read(tmp_pipe[0], &tmp, sizeof(tmp));
            int status;
            std::cout << "Pid " << waitpid(pid, &status, WNOHANG) << std::endl;
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
        if (WIFEXITED(status) || WIFEXITED(status))
            pid = -1;
    }

    void closeStdin() {
        pid_update();
        if (!pid || pid != -1) {
            ::close(pipe_wr_sub[0]);
            ::close(pipe_wr_sub[1]);
        }
    }

    void closeStdout() {
        pid_update();
        if (!pid || pid != -1) {
            ::close(pipe_wr_sub[1]);
            ::close(pipe_wr_sub[0]);
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
        pid_update();
        if (!pid) {
            return ::write(pipe_rd_sub[1], data, len);
        } else if (pid != -1) {
            return ::write(pipe_wr_sub[1], data, len);
        }
        return 0;
    }

    size_t read(void *data, size_t len) {
        pid_update();
        if (!pid) {
            return ::read(pipe_wr_sub[0], data, len);
        } else if (pid != -1) {
            return ::read(pipe_rd_sub[0], data, len);
        }
        return 0;
    }

    void writeExact(const void *data, size_t len) {
        pid_update();
        size_t cur = 0;
        size_t res;
        if (!pid) {
            while (cur < len) {
                res = ::write(pipe_rd_sub[1],
                               (const unsigned char *)data + cur, len - cur);
                if (res <= 0) {
                    return;
                }
                cur += res;
            }
        } else if (pid != -1) {
            while (cur < len) {
                res = ::write(pipe_wr_sub[1],
                               (const unsigned char *)data + cur, len - cur);
                if (res <= 0) {
                    return;
                }
                cur += res;
            }
        }
    }

    void readExact(void *data, size_t len) {
        pid_update();
        size_t cur = 0;
        size_t res;
        if (!pid) {
            while (cur < len) {
                res = ::read(pipe_wr_sub[0],
                               (unsigned char *)data + cur, len - cur);
                if (res <= 0) {
                    return;
                }
                cur += res;
            }
        } else if (pid != -1) {
            while (cur < len) {
                res = ::read(pipe_rd_sub[0],
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
