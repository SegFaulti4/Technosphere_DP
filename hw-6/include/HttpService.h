#ifndef HTTP_HTTPSERVICE_H
#define HTTP_HTTPSERVICE_H

#include "HttpConnection.h"
#include "HttpException.h"
#include "net.h"
#include <thread>

namespace http {

    class IHttpServiceListener {
    public:
        virtual ~IHttpServiceListener() = 0;
        virtual void onRequest(HttpConnection & http_connection) = 0;
    };

    const size_t event_queue_size = 1024;
    const int max_downtime = 5000;
    const int max_connection_amount = 10000;
    const int watchdog_period = 3000;

    using HttpConnectionMap = std::map<int, HttpConnection>;
    using MutexArray = std::array<std::mutex, max_connection_amount>;

    class HttpService {
    private:
        IHttpServiceListener * listener_;
        HttpConnectionMap connections_;
        MutexArray mutexes;
        std::array<bool, max_connection_amount> used_mutexes = { false };
        tcp::Server server_;
        net::Epoll_data epoll_data_;
        net::EPoll accept_epoll_;
        net::EPoll client_epoll_;
        unsigned worker_amount_;
        time_point watchdog_time_point_;
        bool running_ = false;

        void workerRun_();
        void closeConnection_(int fd);
        int findNewMutex_();
        void closeConnection_(HttpConnection & http_con);
        void addConnection_(HttpConnection && http_con);
        int watchdog_time_();
        void watchdog_();

    public:
        explicit HttpService(unsigned worker_amount = 1, IHttpServiceListener * listener = nullptr);
        ~HttpService() = default;

        void setListener(IHttpServiceListener * listener);
        void open(unsigned addr, unsigned port, int max_connection = SOMAXCONN);
        void open(const std::string & addr, unsigned port, int max_connection = SOMAXCONN);
        void close();
        void run();
        void setRunning(bool b);
        bool isRunning() const;
    };

}

#endif //HTTP_HTTPSERVICE_H
