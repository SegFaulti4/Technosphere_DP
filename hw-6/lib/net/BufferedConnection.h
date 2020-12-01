#ifndef NET_BUFFEREDCONNECTION_H
#define NET_BUFFEREDCONNECTION_H

#include "tcp.h"
#include "NetException.h"
#include "EPoll.h"

namespace net {

    class BufferedConnection {
    private:
        int subscription_ = EPOLLRDHUP;
        std::string read_buf_;
        std::string write_buf_;
        tcp::Connection connection_;
        EPoll &epoll_;
        void * epoll_data_;

    public:
        BufferedConnection(tcp::Connection con, EPoll &epoll);
        ~BufferedConnection() = default;
        BufferedConnection(BufferedConnection && other) noexcept ;

        void openEpoll();
        void close();
        void subscribe(EventSubscribe event);
        void unsubscribe(EventSubscribe event);
        void readIntoBuf();
        void writeFromBuf();
        std::string &getReadBuf();
        std::string &getWriteBuf();
        [[nodiscard]] const tcp::Descriptor &getDescriptor() const;
        void ctl(int op, int event);
        void setEpollData(void * ptr);
        BufferedConnection & operator=(BufferedConnection && other) noexcept;
    };

}

#endif //NET_BUFFEREDCONNECTION_H
