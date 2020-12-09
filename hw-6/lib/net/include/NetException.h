#ifndef NET_NETEXCEPTION_H
#define NET_NETEXCEPTION_H

#include <stdexcept>
#include "TcpException.h"

namespace net {

    class NetException : public tcp::TcpException {
    public:
        using tcp::TcpException::TcpException;
    };

}

#endif //NET_NETEXCEPTION_H
