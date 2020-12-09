#ifndef HTTP_HTTPCONNECTION_H
#define HTTP_HTTPCONNECTION_H

#include "net.h"
#include <map>
#include <chrono>
#include <mutex>

namespace http {

    using steady_clock = std::chrono::steady_clock;
    using time_point = std::chrono::time_point<steady_clock>;
    using HttpRequest = std::map<std::string, std::string>;
    using ms = std::chrono::milliseconds;

    class HttpConnection : private net::BufferedConnection {
    private:
        enum class HttpParserMode {
            REQUEST_LINE = 0,
            MESSAGE_HEADERS = 1,
            MESSAGE_BODY = 2
        };

        HttpRequest request_;
        bool request_available_ = false;
        bool watchdog_refresh_flag_ = false;
        HttpParserMode mode_ = HttpParserMode::REQUEST_LINE;
        time_point last_used_ = steady_clock::now();
        std::mutex mutex_;

    public:
        HttpConnection(tcp::Connection && con, net::EPoll &epoll);
        HttpConnection(HttpConnection && other) noexcept;
        ~HttpConnection() = default;

        void close();
        void setEpollData(void * ptr);
        void openEpoll();
        void readUntilEagain();
        void writeUntilEagain();
        void unsubscribe(net::EventSubscribe event);
        void resubscribe();
        void refreshTime();
        int downtimeDuration();
        bool isWriting();
        bool isValid();
        bool refreshIsDelayed();
        void setDelayedRefresh();
        bool requestAvailable() const;
        std::mutex & getMutex();
        const tcp::Descriptor & getDescriptor() const;

        HttpRequest & getRequest();
        void clearRequest();
        void writeResponse(const std::string & response);

        HttpConnection & operator=(HttpConnection && other) noexcept;
    };

}

#endif //HTTP_HTTPCONNECTION_H
