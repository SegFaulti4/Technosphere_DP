#include "net.h"
#include <iostream>

class HelloWorldService : public net::IServiceListener {
public:
    void onNewConnection(net::BufferedConnection & buf_con) {
        buf_con.subscribe(net::READ);
    }

    virtual void onClose(net::BufferedConnection & buf_con) {
        std::cout << "Connection closed" << std::endl;
    }

    virtual void onWriteDone(net::BufferedConnection & buf_con) {}

    virtual void onReadAvailable(net::BufferedConnection & buf_con) {
        buf_con.buf_read();
        std::cout << buf_con.get_read_buf() << std::endl;
    }

    virtual void onError(net::BufferedConnection & buf_con, const std::string & what) {
        std::cout << what << std::endl;
    }
};

int main() {
    net::Service service(std::make_unique<HelloWorldService>());
    service.open(INADDR_ANY, 8080);
    service.run();
    return 0;
}
