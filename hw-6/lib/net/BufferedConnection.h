#ifndef NET_BUFFEREDCONNECTION_H
#define NET_BUFFEREDCONNECTION_H

#include "tcp.h"
#include "NetException.h"
#include "EPoll.h"

namespace net {

    class BufferedConnection {
    private:
        static const size_t max_read_length_ = 4096;
        int subscription_ = EPOLLRDHUP;
        std::string read_buf_;
        std::string write_buf_;
        tcp::Connection connection_;
        EPoll &epoll_;
        Epoll_data epoll_data_;

    public:
        BufferedConnection(tcp::Connection con, EPoll &epoll);
        ~BufferedConnection() = default;
        BufferedConnection(BufferedConnection && other) noexcept ;

        void subscribe(Event_subscribe event);
        void unsubscribe(Event_subscribe event);
        void read_into_buf();
        void write_from_buf();
        std::string &get_read_buf();
        std::string &get_write_buf();
        const tcp::Descriptor &get_descriptor() const;
        void ctl(int op, int event);
        void set_meta(int meta);
        void set_ptr(void * ptr);
        BufferedConnection & operator=(BufferedConnection && other) noexcept;
    };

}

#endif //NET_BUFFEREDCONNECTION_H
