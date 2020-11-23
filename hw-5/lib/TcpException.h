#ifndef TCP_TCPEXCEPTION_H
#define TCP_TCPEXCEPTION_H

#include <string>
#include <utility>
#include <stdexcept>

namespace tcp {

    class TcpException : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    class TcpBlockException : public TcpException {
    public:
        using TcpException::TcpException;
    };

}

#endif //TCP_TCPEXCEPTION_H
