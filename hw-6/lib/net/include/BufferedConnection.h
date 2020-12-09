#ifndef NET_BUFFEREDCONNECTION_H
#define NET_BUFFEREDCONNECTION_H

#include "tcp.h"
#include "EPoll.h"

namespace net {

    enum class EventSubscribe {
        NOTHING = 0,
        READ = EPOLLIN,
        WRITE = EPOLLOUT,
        READWRITE = EPOLLIN | EPOLLOUT
    };

    class BufferedConnection {
    protected:
        int subscription_ = EPOLLRDHUP;
        std::string read_buf_;
        std::string write_buf_;
        tcp::Connection connection_;
        EPoll &epoll_;
        void * epoll_data_;

    public:
        BufferedConnection(tcp::Connection && con, EPoll &epoll);
        ~BufferedConnection() = default;
        BufferedConnection(BufferedConnection && other) noexcept;

        void setEpollData(void * ptr);
        void openEpoll();
        void close();
        void subscribe(EventSubscribe event);
        void unsubscribe(EventSubscribe event);
        void readIntoBuf();
        void writeFromBuf();
        std::string &getReadBuf();
        std::string &getWriteBuf();
        [[nodiscard]] const tcp::Descriptor &getDescriptor() const;
        BufferedConnection & operator=(BufferedConnection && other) noexcept;
    };

}

#endif //NET_BUFFEREDCONNECTION_H
