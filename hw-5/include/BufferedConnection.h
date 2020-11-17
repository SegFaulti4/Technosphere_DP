#ifndef NET_BUFFEREDCONNECTION_H
#define NET_BUFFEREDCONNECTION_H

#include "tcp.h"
#include "NetException.h"
#include "EPoll.h"

namespace net {

    class Service;

    class BufferedConnection {
    private:
        static const size_t max_read_size_ = 4096;
        char tmp_[max_read_size_] = {0};
        int subscription_ = EPOLLRDHUP;
        std::string read_buf_;
        std::string write_buf_;
        tcp::Connection connection_;
        EPoll &epoll_;
        Service &service_;

    public:
        BufferedConnection(tcp::Connection con, EPoll &epoll, Service &service);
        ~BufferedConnection();

        void subscribe(Event_subscribe event);
        void unsubscribe(Event_subscribe event);
        void buf_read();
        void buf_write();
        std::string &get_read_buf();
        std::string &get_write_buf();
        void close();
        const tcp::Descriptor &get_descriptor();

        BufferedConnection &operator=(BufferedConnection &&other) noexcept;
    };

}

#endif //NET_BUFFEREDCONNECTION_H
