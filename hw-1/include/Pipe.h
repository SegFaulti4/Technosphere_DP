#ifndef HW_1_PIPE_H
#define HW_1_PIPE_H

class Pipe{
private:
    int fd_[2];

public:
    explicit Pipe(int flag = 0);
    ~Pipe();

    int rd();
    int wr();
    void close_rd();
    void close_wr();
    void close();
};

#endif //HW_1_PIPE_H
