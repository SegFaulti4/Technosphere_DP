#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "Descriptor.h"
#include <string>

namespace tcp {

    class Connection {
    private:
        Descriptor descriptor_;
        explicit Connection(int socket);
        void setTimeout(ssize_t ms, int opt);
        void connect_(unsigned addr, unsigned port); // перекрывает публичный метод

    public:
        Connection() = default;
        Connection(const std::string & addr, unsigned port);
        Connection(unsigned addr, unsigned port);

        void connect(const std::string & addr, unsigned port);
        void connect(unsigned addr, unsigned port);
        void close();
        size_t read(void *buf, size_t count);
        size_t write(const void *buf, size_t count);
        void readExact(void *buf, size_t count);
        void writeExact(const void *buf, size_t count);
        void setTimeout(ssize_t ms);
        [[nodiscard]] const Descriptor & getDescriptor() const;

        friend class Server;
        /*
            Server используем приватный конструктор Connection,
            принимающий на вход сокет, давать пользователю в руки
            такой метод будет небезопасно.
        */
    };

}

#endif //TCP_CONNECTION_H
