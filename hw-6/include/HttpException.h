#ifndef NET_NETEXCEPTION_H
#define NET_NETEXCEPTION_H

#include <stdexcept>

namespace http {

    class HttpException : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

}

#endif //NET_NETEXCEPTION_H
