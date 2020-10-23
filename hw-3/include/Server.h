#ifndef HW_3_SERVER_H
#define HW_3_SERVER_H

#include "Connection.h"

namespace tcp {

    class Server {
    private:
        Descriptor dscrptr_;
        sockaddr_in addr_in_ = {};

    public:
        Server();
        Server(Server && other) noexcept;
        Server(const std::string addr, unsigned port,
               unsigned max_connection = SOMAXCONN);

        void listen(const std::string addr, unsigned port,
                  unsigned max_connection = SOMAXCONN);
        void close();
        Connection accept();
        void set_timeout(ssize_t ms);
        void set_max_connection(unsigned new_max);
        Server & operator=(Server && other) noexcept;
    };

}

#endif //HW_3_SERVER_H
