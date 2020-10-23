#ifndef HW_3_CONNECTION_H
#define HW_3_CONNECTION_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string>
#include "Descriptor.h"

namespace tcp {

    class Connection {
    private:
        Descriptor dscrptr_;
        Connection(int socket);

    public:
        Connection();
        explicit Connection(const std::string & addr, unsigned port);
        Connection(Connection && other) noexcept;

        Connection& operator=(Connection && other) noexcept;
        void connect(const std::string & addr, unsigned port);
        void close();
        ssize_t read(void *buf, size_t count);
        ssize_t write(const void *buf, size_t count);
        void readExact(void *buf, size_t count);
        void writeExact(const void *buf, size_t count);
        void set_timeout(ssize_t ms);

        friend class Server;
    };

}

#endif //HW_3_CONNECTION_H
