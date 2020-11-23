#ifndef HW_3_CONNECTION_H
#define HW_3_CONNECTION_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string>
#include <climits>
#include "TcpException.h"
#include "Descriptor.h"

namespace tcp {

    class Connection {
    private:
        Descriptor descriptor_;
        explicit Connection(int socket);
        void set_timeout_(ssize_t ms, int opt);
        void connect_(unsigned addr, unsigned port);

    public:
        Connection();
        Connection(const std::string & addr, unsigned port);
        Connection(unsigned addr, unsigned port);
        Connection(Connection && other) noexcept;
        ~Connection() = default;

        Connection& operator=(Connection && other) noexcept;
        void connect(const std::string & addr, unsigned port);
        void connect(unsigned addr, unsigned port);
        void close();
        size_t read(void *buf, size_t count);
        size_t write(const void *buf, size_t count);
        void readExact(void *buf, size_t count);
        void writeExact(const void *buf, size_t count);
        void set_timeout(ssize_t ms);
        [[nodiscard]] const Descriptor & get_descriptor() const;

        friend class Server;
        /*
            Server используем приватный конструктор Connection,
            принимающий на вход сокет, давать пользователю в руки
            такой метод будет небезопасно.
        */
    };

}

#endif //HW_3_CONNECTION_H
