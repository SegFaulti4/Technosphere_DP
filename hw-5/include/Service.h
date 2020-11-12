#ifndef HW_5_SERVICE_H
#define HW_5_SERVICE_H

#include <memory>
#include <map>
#include <vector>
#include "tcp.h"
#include <utility>
#include "NetException.h"
#include "EPoll.h"

namespace net {

    class Service;

    class BufferedConnection {
    private:
        static const size_t min_read_size_ = 4096;
        char tmp_[min_read_size_] = { 0 };
        int subscription_ = EPOLLRDHUP;
        std::string read_buf_;
        std::string write_buf_;
        tcp::Connection connection_;
        EPoll & epoll_;
        Service & service_;

    public:
        BufferedConnection(tcp::Connection con, EPoll & epoll, Service & service);
        ~BufferedConnection();

        BufferedConnection& operator=(BufferedConnection && other) noexcept;
        void subscribe(Event_subscribe event);
        void unsubscribe(Event_subscribe event);
        void buf_read();
        void buf_write();
        std::string & get_read_buf();
        std::string & get_write_buf();
        void close();
        const tcp::Descriptor & get_descriptor();
    };

    class IServiceListener {
    public:
        virtual void onNewConnection(BufferedConnection & buf_con) = 0;
        virtual void onClose(BufferedConnection & buf_con) = 0;
        virtual void onWriteDone(BufferedConnection & buf_con) = 0;
        virtual void onReadAvailable(BufferedConnection & buf_con) = 0;
        virtual void onError(BufferedConnection & buf_con, const std::string & what) = 0;
    };

    const size_t event_queue_size_ = 1024;

    class Service {
    private:
        std::unique_ptr<IServiceListener> listener_;
        std::map<int, std::unique_ptr<BufferedConnection>> connections_;
        std::unique_ptr<tcp::Server> server_;
        EPoll epoll_;

        void closeConnection_(int fd);

    public:
        explicit Service(std::unique_ptr<IServiceListener> listener = nullptr);
        ~Service();

        void set_listener(std::unique_ptr<IServiceListener> listener);
        void open(unsigned addr, unsigned port, int max_connection = SOMAXCONN);
        void open(const std::string & addr, unsigned port, int max_connection = SOMAXCONN);
        void close();
        void run();
        void closeConnection(BufferedConnection & buf_con);
    };

}

#endif //HW_5_SERVICE_H
