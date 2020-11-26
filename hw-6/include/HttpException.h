#ifndef HTTP_HTTPEXCEPTION_H
#define HTTP_HTTPEXCEPTION_H

#include <stdexcept>
#include "NetException.h"

namespace http {

    class HttpException : public net::NetException {
        using net::NetException::NetException;
    };

}

#endif //HTTP_HTTPEXCEPTION_H
