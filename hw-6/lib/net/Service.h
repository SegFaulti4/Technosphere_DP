#ifndef NET_SERVICE_H
#define NET_SERVICE_H

#include <memory>
#include <map>
#include "tcp.h"
#include "NetException.h"
#include "BufferedConnection.h"
#include "EPoll.h"

namespace net {

    class Service;

    class IServiceListener {
    public:
        virtual ~IServiceListener() = 0;
        virtual void onNewConnection(BufferedConnection && buf_con, Service & service) = 0;
        virtual void onClose(BufferedConnection & buf_con, Service & service) = 0;
        virtual void onWriteDone(BufferedConnection & buf_con, Service & service) = 0;
        virtual void onReadAvailable(BufferedConnection & buf_con, Service & service) = 0;
        virtual void onError(BufferedConnection & buf_con, const std::string & what, Service & service) = 0;
    };

    const size_t event_queue_size_ = 1024;

    using BufferedConnectionMap = std::map<int, BufferedConnection>;

    class Service {
    private:
        IServiceListener * listener_;
        BufferedConnectionMap connections_;
        tcp::Server server_;
        EPoll epoll_;
        Epoll_data epoll_data_;
        bool running_ = false;

        void closeConnection_(int fd);

    public:
        explicit Service(IServiceListener * listener = nullptr);
        ~Service() = default;

        void setListener(IServiceListener * listener);
        void open(unsigned addr, unsigned port, int max_connection = SOMAXCONN);
        void open(const std::string & addr, unsigned port, int max_connection = SOMAXCONN);
        void close();
        void run();
        void closeConnection(BufferedConnection & buf_con);
        void addConnection(BufferedConnection && buf_con);
        void setRunning(bool b);
        bool isRunning() const;
        Service & operator=(Service && other) noexcept;
    };

}

#endif //NET_SERVICE_H
