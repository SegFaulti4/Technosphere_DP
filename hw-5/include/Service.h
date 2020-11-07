#ifndef HW_5_SERVICE_H
#define HW_5_SERVICE_H

#include <memory>
#include <map>
#include <vector>
#include "tcp.h"
#include "NetException.h"
#include "EPoll.h"
#include "IServiceListener.h"
#include "BufferedConnection.h"

namespace net {

    class Service {
    private:
        std::unique_ptr<IServiceListener> listener_;
        std::map<int, std::unique_ptr<BufferedConnection>> connections_;
        std::unique_ptr<tcp::Server> server_;
        EPoll epoll_;

        void closeConnection_(int fd);

    public:
        explicit Service(std::unique_ptr<IServiceListener> listener = nullptr);

        void set_listener(std::unique_ptr<IServiceListener> listener);
        void open(unsigned addr, unsigned port, int max_connection = SOMAXCONN);
        void open(const std::string & addr, unsigned port, int max_connection = SOMAXCONN);
        void close();
        void run();
        void closeConnection(BufferedConnection & buf_con);
        static void subscribeTo(BufferedConnection & buf_con, Event_subscribe event);
        static void unsubscribeFrom(BufferedConnection & buf_con, Event_subscribe event);
    };

}

#endif //HW_5_SERVICE_H
