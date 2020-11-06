#include "tcp.h"
#include <iostream>

int main() {
    try {
        tcp::Connection con1(INADDR_LOOPBACK, 8080);
        //con1.set_timeout(1000);
        tcp::Connection con2(std::move(con1));
        tcp::Connection con;
        con = std::move(con2);
        char buf[1024] = {};
        ssize_t res;
        while ((res = con.read(buf, 1023)) > 0) {
            std::cout << std::string(buf, res) << std::flush;
        }
    } catch (tcp::TcpException & exc) {
        std::cout << exc.getError() << std::endl;
    }
    return 0;
}
