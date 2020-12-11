#include "http.h"
#include "log.h"

class ReadService : public http::IHttpServiceListener {
public:
    ~ReadService() = default;

    void onRequest(http::HttpConnection & con) {
        log::info("request");
        http::HttpRequest request = con.getRequest();
        /*for (const auto& it : request) {
            std::cout << it.first << ": " << it.second << std::endl;
        }*/
        con.writeResponse("HTTP/1.1 200 OK\r\n\r\n");
    }
};

http::IHttpServiceListener::~IHttpServiceListener() = default;

int main() {
    log::initWithStderrLogger(log::LL_INFO);
    ReadService listener;
    http::HttpService service(1, &listener);
    service.open(INADDR_ANY, 8080);
    service.run();
    return 0;
}