#ifndef HTTP_HTTPCONNECTION_H
#define HTTP_HTTPCONNECTION_H

#include "net.h"
#include "log.h"
#include "HttpException.h"
#include <map>
#include <vector>
#include <chrono>

namespace http {

    using steady_clock = std::chrono::steady_clock;
    using time_point = std::chrono::time_point<steady_clock>;
    using HttpRequest = std::map<std::string, std::string>;
    using ms = std::chrono::milliseconds;

    class HttpConnection {
    private:
        enum HttpParserMode {
            REQUEST_LINE = 0,
            MESSAGE_HEADERS = 1,
            MESSAGE_BODY = 2
        };

        net::BufferedConnection connection_;
        HttpRequest request_;
        bool request_available_ = false;
        HttpParserMode mode_ = REQUEST_LINE;
        time_point last_used_ = steady_clock::now();
        int subscription_ = EPOLLET | EPOLLONESHOT | EPOLLIN | EPOLLRDHUP;
        std::mutex mutex_;

    public:
        explicit HttpConnection(net::BufferedConnection && buf_con);
        ~HttpConnection() = default;
        HttpConnection(HttpConnection && other) noexcept;

        void openEpoll();
        void close();
        void readUntilEagain();
        void writeUntilEagain();
        void subscribe(net::EventSubscribe event);
        void unsubscribe(net::EventSubscribe event);
        void resubscribe();
        void refreshTime();
        int downtimeDuration();
        bool isWriting();
        bool isValid();
        bool requestAvailable() const;
        const tcp::Descriptor & getDescriptor() const;
        std::mutex & getMutex();

        HttpRequest & getRequest();
        void clearRequest();
        void writeResponse(const std::string & response);

        HttpConnection & operator=(HttpConnection && other) noexcept;
    };

}

#endif //HTTP_HTTPCONNECTION_H
