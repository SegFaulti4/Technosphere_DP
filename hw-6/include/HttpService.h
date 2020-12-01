#ifndef HTTP_HTTPSERVICE_H
#define HTTP_HTTPSERVICE_H

#include "HttpConnection.h"
#include "HttpException.h"
#include "net.h"
#include "log.h"
#include <thread>
#include <queue>
#include <atomic>
#include <list>

namespace http {

    class IHttpServiceListener {
    public:
        virtual ~IHttpServiceListener() = 0;
        virtual void onRequest(HttpConnection & http_connection) = 0;
    };

    using HttpConnectionList = std::list<HttpConnection>;
    using HttpConnectionQueue = std::queue<HttpConnection *>;
    using steady_clock = std::chrono::steady_clock;
    using time_point = std::chrono::time_point<steady_clock>;
    using HttpRequest = std::map<std::string, std::string>;
    using ms = std::chrono::milliseconds;

    class HttpService {
    private:
        IHttpServiceListener * listener_;
        HttpConnectionList connections_;
        HttpConnectionQueue available_connections_;
        tcp::Server server_;
        net::EPoll client_epoll_;
        std::mutex queue_mutex_;
        unsigned worker_amount_;
        bool running_ = false;
        std::atomic<int> opened_connections_amount_ = 0;
        time_point last_watchdog_run_ = steady_clock::now();

        void workerRun_();
        void closeConnection_(HttpConnection & http_con);
        void addConnection_(HttpConnection && http_con);
        void watchdog_();
        void setRunning_(bool b);
        int watchdogDowntimeDuration_();

    public:
        explicit HttpService(unsigned worker_amount = 1, IHttpServiceListener * listener = nullptr);
        ~HttpService() = default;

        void setListener(IHttpServiceListener * listener);
        void open(unsigned addr, unsigned port, int max_connection = SOMAXCONN);
        void open(const std::string & addr, unsigned port, int max_connection = SOMAXCONN);
        void close();
        void run();
        bool isRunning() const;
    };

}

#endif //HTTP_HTTPSERVICE_H
