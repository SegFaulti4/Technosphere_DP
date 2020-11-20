#include <iostream>
#include "http.h"

class ReadService : public http::HttpServiceListener {
public:
    void onRequest(http::HttpConnection & buf_con) {
        std::cout << buf_con.get_read_buf() << std::endl;
    }
};

int main() {
    http::Service service(1, std::make_unique<ReadService>());
    service.open(INADDR_ANY, 8080);
    service.run();
    return 0;
}
