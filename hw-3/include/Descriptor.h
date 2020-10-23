#ifndef HW_3_DESCRIPTOR_H
#define HW_3_DESCRIPTOR_H

#include <iostream>
#include <cstring>
#include <stdexcept>
#include "unistd.h"

namespace tcp {

    class Descriptor {
    private:
        int fd_;

    public:
        explicit Descriptor(int fd = -1);
        Descriptor(Descriptor && other) noexcept;
        ~Descriptor();

        void close();
        void set_fd(int fd);
        int get_fd() const;
        ssize_t read(void *buf, size_t count) const;
        ssize_t write(const void *buf, size_t count) const;
        void readExact(void *buf, size_t count) const;
        void writeExact(const void *buf, size_t count) const;
        Descriptor & operator=(Descriptor && other) noexcept;
    };

}

#endif //HW_3_DESCRIPTOR_H
