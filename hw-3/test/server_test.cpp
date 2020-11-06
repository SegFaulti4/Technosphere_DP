#include "tcp.h"
#include <iostream>

int main() {
    try {
        tcp::Server server1(INADDR_ANY, 8080);
        tcp::Server server2(std::move(server1));
        tcp::Server server;
        server = std::move(server2);
        //server.set_timeout(1000);
        tcp::Connection con(std::move(server.accept()));
        std::string str = "Hello World!";
        con.write(str.data(), str.size());
    } catch (tcp::TcpException & exc) {
        std::cout << exc.getError() << std::endl;
    }
    return 0;
}
