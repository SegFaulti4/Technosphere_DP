#ifndef HW_1_FILEDES_H
#define HW_1_FILEDES_H

#include <string>
#include "fcntl.h"

namespace log {

    class FileDes {
    private:
        int fd_;

    public:
        explicit FileDes(int fd = -1);

        explicit FileDes(const std::string &path, int flags, mode_t);

        ~FileDes();

        void close();

        void open(const std::string &path, int flags, mode_t = 0);

        size_t read(void *buf, size_t count);

        size_t write(const void *buf, size_t count);

        void replace(int fd);

        void flush();
    };

}

#endif //HW_1_FILEDES_H
