#ifndef TCP_TCPEXCEPTION_H
#define TCP_TCPEXCEPTION_H

#include <string>
#include <utility>

namespace tcp {

    class TcpException : std::exception {
    private:
        std::string m_error;

    public:
        explicit TcpException(std::string error) : m_error(std::move(error)) {};

        const char *getError() { return m_error.data(); }
    };

}

#endif //TCP_TCPEXCEPTION_H
