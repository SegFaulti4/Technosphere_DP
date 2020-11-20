#ifndef HW_6_SERVICE_H
#define HW_6_SERVICE_H

#include <memory>
#include <map>
#include <thread>
#include <vector>
#include <functional>
#include <ratio>
#include "tcp.h"
#include "HttpException.h"
#include "HttpConnection.h"
#include "EPoll.h"

namespace http {

    using ConnectionMap = std::map<int, std::unique_ptr<HttpConnection>>;

    class HttpServiceListener {
    public:
        virtual void onRequest(HttpConnection & buf_con) = 0;
    };

    const size_t event_queue_size = 1024;
    const int max_downtime = 5000;

    class Service {
    private:
        std::unique_ptr<HttpServiceListener> listener_;
        ConnectionMap connections_;
        std::unique_ptr<tcp::Server> server_;
        EPoll accept_epoll_;
        EPoll client_epoll_;
        unsigned worker_amount_;

        void closeConnection_(int fd);
        void worker_run_();

    public:
        explicit Service(unsigned worker_amount = 1, std::unique_ptr<HttpServiceListener> listener = nullptr);
        ~Service();

        void set_listener(std::unique_ptr<HttpServiceListener> listener);
        void open(unsigned addr, unsigned port, int max_connection = SOMAXCONN);
        void open(const std::string & addr, unsigned port, int max_connection = SOMAXCONN);
        void close();
        void run();
        void closeConnection(HttpConnection & buf_con);
    };

}

#endif //HW_6_SERVICE_H
