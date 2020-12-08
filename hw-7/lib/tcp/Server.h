#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "Connection.h"
#include "TcpException.h"

namespace tcp {

    class Server {
    private:
        Descriptor descriptor_;
        void setTimeout_(ssize_t ms, int opt);
        void listen_(unsigned addr, unsigned port, int max_connection);

    public:
        Server();
        Server(Server && other) noexcept;
        Server(const std::string & addr, unsigned port,
               int max_connection = SOMAXCONN);
        Server(unsigned addr, unsigned port, int max_connection = SOMAXCONN);

        void listen(const std::string & addr, unsigned port,
                  int max_connection = SOMAXCONN);
        void listen(unsigned addr, unsigned port, int max_connection = SOMAXCONN);
        void close();
        Connection accept();
        void setTimeout(ssize_t ms);
        void setMaxConnection(int new_max);
        Server & operator=(Server && other) noexcept;
        const Descriptor & getDescriptor();
    };

}

#endif //TCP_SERVER_H
