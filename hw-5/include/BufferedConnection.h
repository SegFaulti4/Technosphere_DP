#ifndef HW_5_BUFFEREDCONNECTION_H
#define HW_5_BUFFEREDCONNECTION_H

#include <utility>
#include <vector>
#include "tcp.h"
#include "EPoll.h"

namespace net {

    class BufferedConnection {
    private:
        static const size_t min_read_size_ = 4096;
        char tmp_[min_read_size_] = { 0 };
        int subscription_ = EPOLLRDHUP;
        std::string read_buf_;
        std::string write_buf_;
        tcp::Connection connection_;
        EPoll & epoll_;

    public:
        explicit BufferedConnection(tcp::Connection con, EPoll & epoll);

        BufferedConnection& operator=(BufferedConnection && other) noexcept;
        void subscribe(Event_subscribe event);
        void unsubscribe(Event_subscribe event);
        void buf_read();
        void buf_write();
        std::string & get_read_buf();
        std::string & get_write_buf();
        const tcp::Descriptor & get_descriptor();
        void close();
    };

}

#endif //HW_5_BUFFEREDCONNECTION_H
