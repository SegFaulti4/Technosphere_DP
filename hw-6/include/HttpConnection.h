#ifndef NET_BUFFEREDCONNECTION_H
#define NET_BUFFEREDCONNECTION_H

#include "tcp.h"
#include "HttpException.h"
#include "EPoll.h"
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
        static const size_t max_read_size_ = 4096;
        char tmp_[max_read_size_] = {0};
        int subscription_ = EPOLLRDHUP | EPOLLET | EPOLLONESHOT;
        std::string read_buf_;
        std::string write_buf_;
        tcp::Connection connection_;
        EPoll &epoll_;
        std::vector<HttpRequest> request_queue_;
        time_point last_used_ = steady_clock::now();

        void subscribe(Event_subscribe event);
        void resubscribe();
        const tcp::Descriptor &get_descriptor();
        bool request_available();
        bool write_ongoing();
        void refresh_time();
        int downtime_duration();
        void buf_read();
        void buf_write();
        void read_until_eagain();
        void write_until_eagain();

    public:
        HttpConnection(tcp::Connection con, EPoll &epoll);
        ~HttpConnection();

        std::string &get_read_buf();

        HttpConnection &operator=(HttpConnection &&other) noexcept;

        friend class Service;
    };

}

#endif //NET_BUFFEREDCONNECTION_H
