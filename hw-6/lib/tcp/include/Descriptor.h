#ifndef TCP_DESCRIPTOR_H
#define TCP_DESCRIPTOR_H

#include <sys/types.h>

namespace tcp {

    class Descriptor {
    private:
        int fd_ = -1;

    public:
        Descriptor() = default;
        explicit Descriptor(int fd);
        Descriptor(Descriptor &&other) noexcept;
        ~Descriptor();

        void close();
        void setFd(int fd);
        [[nodiscard]] int getFd() const;
        size_t read(void *buf, size_t count) const;
        size_t write(const void *buf, size_t count) const;
        void readExact(void *buf, size_t count) const;
        void writeExact(const void *buf, size_t count) const;
        Descriptor &operator=(Descriptor &&other) noexcept;
        [[nodiscard]] bool isValid() const;
    };

}

#endif //TCP_DESCRIPTOR_H
