#include <iostream>
#include "http.h"

class ReadService : public http::IHttpServiceListener {
public:
    void onRequest(http::HttpConnection & con) {
        http::HttpRequest & request = con.get_request();
        con.clear_request();
        for (auto it : request) {
            std::cout << it.first << ": " << it.second << std::endl;
        }
    }
};

int main() {
    ReadService listener;
    http::HttpService service(1, &listener);
    service.open(INADDR_ANY, 8080);
    service.run();
    return 0;
}