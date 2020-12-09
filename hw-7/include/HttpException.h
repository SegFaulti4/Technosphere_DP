#ifndef HTTP_HTTPEXCEPTION_H
#define HTTP_HTTPEXCEPTION_H

#include "NetException.h"

namespace http {

    class HttpException : public net::NetException {
        using net::NetException::NetException;
    };

    class HttpOnCloseException : public HttpException {
        using HttpException::HttpException;
    };

}

#endif //HTTP_HTTPEXCEPTION_H
