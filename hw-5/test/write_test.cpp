#include "net.h"
#include <iostream>

class WriteService : public net::IServiceListener {
public:
    void onNewConnection(net::BufferedConnection & buf_con) {
        buf_con.subscribe(net::WRITE);
        buf_con.get_write_buf() = "Hello World!";
    }

    virtual void onClose(net::BufferedConnection & buf_con) {
        std::cout << "Connection closed" << std::endl;
    }

    virtual void onWriteDone(net::BufferedConnection & buf_con) {
        buf_con.close();
        std::cout << "Connection closed" << std::endl;
    }

    virtual void onReadAvailable(net::BufferedConnection & buf_con) {}

    virtual void onError(net::BufferedConnection & buf_con, const std::string & what) {
        std::cout << what << std::endl;
    }
};

int main() {
    net::Service service(std::make_unique<WriteService>());
    service.open(INADDR_ANY, 8080);
    service.run();
    return 0;
}
