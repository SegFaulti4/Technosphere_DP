#ifndef HW_1_DESCRIPTOR_H
#define HW_1_DESCRIPTOR_H

#include <string>
#include "fcntl.h"
#include <stdexcept>
#include <utility>
#include "unistd.h"
#include "TcpException.h"

namespace tcp {

    class Descriptor {
    private:
        int fd_ = -1;

    public:
        Descriptor();
        explicit Descriptor(int fd);
        Descriptor(Descriptor &&other) noexcept;
        ~Descriptor();

        void close();
        void set_fd(int fd);
        int get_fd() const;
        size_t read(void *buf, size_t count) const;
        size_t write(const void *buf, size_t count) const;
        void readExact(void *buf, size_t count) const;
        void writeExact(const void *buf, size_t count) const;
        Descriptor &operator=(Descriptor &&other) noexcept;
        bool is_valid() const;
    };

}

#endif //HW_1_DESCRIPTOR_H
