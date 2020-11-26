#ifndef HTTP_HTTPCONNECTION_H
#define HTTP_HTTPCONNECTION_H

#include "net.h"
#include "HttpException.h"
#include <map>
#include <vector>
#include <chrono>

namespace http {

    using steady_clock = std::chrono::steady_clock;
    using time_point = std::chrono::time_point<steady_clock>;
    using HttpRequest = std::map<std::string, std::string>;
    using ms = std::chrono::milliseconds;

    enum HttpParserMode {
        REQUEST_LINE = 0,
        MESSAGE_HEADERS = 1,
        MESSAGE_BODY = 2
    };

    const std::string AllowedRequestMethods[] = {
            "GET"
    };

    const std::string AllowedType_Versions[] = {
            "HTTP/1.1",
            "HTTP/1.0"
    };

    const std::string AllowedMessageHeaders[] = {
            "Connection: ",
            "Content-Length: "
    };

    const int max_request_line_length = 4096;
    const int max_message_headers_length = 4096;
    const int max_message_body_length = 4096;

    class HttpConnection {
    private:
        net::BufferedConnection connection_;
        HttpRequest request_;
        bool request_available_ = false;
        bool valid_ = true;
        HttpParserMode mode_ = REQUEST_LINE;
        time_point last_used_ = steady_clock::now();
        int subscription_ = EPOLLRDHUP | EPOLLET | EPOLLONESHOT | EPOLLIN;
        int mutex_idx_;

    public:
        explicit HttpConnection(net::BufferedConnection && buf_con, int mutex_idx);
        ~HttpConnection() = default;
        HttpConnection(HttpConnection && other) noexcept;

        void read_until_eagain();
        void write_until_eagain();
        bool request_available() const;
        HttpRequest & get_request();
        void clear_request();
        int downtime_duration();
        void refresh_time();
        void resubscribe();
        void subscribe(net::Event_subscribe event);
        void unsubscribe(net::Event_subscribe event);
        bool write_ongoing();
        bool is_valid() const;
        void set_valid(bool b);
        const tcp::Descriptor & getDescriptor() const;
        std::string &get_read_buf();
        int get_mutex_idx() const;
        HttpConnection & operator=(HttpConnection && other) noexcept;

        friend class HttpService;
    };

}

#endif //HTTP_HTTPCONNECTION_H
