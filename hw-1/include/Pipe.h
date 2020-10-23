#ifndef HW_1_PIPE_H
#define HW_1_PIPE_H

#include "Descriptor.h"
#include <stdexcept>
#include "unistd.h"

class Pipe{
private:
    Descriptor rd_, wr_;

public:
    explicit Pipe(int flag = 0);

    int rd();
    int wr();
    void close_rd();
    void close_wr();
    void close();
};

#endif //HW_1_PIPE_H
