#ifndef HW_1_PROCESS_H
#define HW_1_PROCESS_H

#include "Pipe.h"
#include "Descriptor.h"
#include <stdexcept>
#include <sys/wait.h>
#include "unistd.h"
#include "fcntl.h"

class Process {
private:
    Descriptor rd_, wr_;
    pid_t pid_;

public:
    explicit Process(const std::string& path);
    ~Process();

    void pid_update();
    void closeStdin();
    void closeStdout();
    void close();
    size_t write(const void *data, size_t len);
    size_t read(void *data, size_t len);
    void writeExact(const void *data, size_t len);
    void readExact(void *data, size_t len);
};

#endif //HW_1_PROCESS_H
