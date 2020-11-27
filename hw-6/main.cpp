#include <iostream>
#include "http.h"
#include "log.h"

class ReadService : public http::IHttpServiceListener {
public:
    ~ReadService() = default;

    void onRequest(http::HttpConnection & con) {
        log::info("request");
        http::HttpRequest & request = con.get_request();
        for (const auto& it : request) {
            std::cout << it.first << ": " << it.second << std::endl;
        }
        con.clear_request();
        con.write_response("HTTP/1.1 200 OK\r\n\r\n");
    }
};

http::IHttpServiceListener::~IHttpServiceListener() = default;

int main() {
    log::init_with_stderr_logger(log::LL_INFO);
    ReadService listener;
    http::HttpService service(2, &listener);
    service.open(INADDR_ANY, 8080);
    service.run();
    return 0;
}