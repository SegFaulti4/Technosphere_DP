#ifndef HW_3_SERVER_H
#define HW_3_SERVER_H

#include "Connection.h"
#include "TcpException.h"

namespace tcp {

    class Server {
    private:
        Descriptor dscrptr_;
        sockaddr_in addr_in_ = {};
        void set_timeout_(ssize_t ms, int opt);
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
        void set_timeout(ssize_t ms);
        void set_max_connection(int new_max);
        Server & operator=(Server && other) noexcept;
        const Descriptor & get_descriptor();
    };

}

#endif //HW_3_SERVER_H
