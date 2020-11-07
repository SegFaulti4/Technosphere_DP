#ifndef HW_5_ISERVICELISTENER_H
#define HW_5_ISERVICELISTENER_H

#include "BufferedConnection.h"

namespace net {

    class IServiceListener {
    public:
        virtual void onNewConnection(BufferedConnection & buf_con) = 0;
        virtual void onClose(BufferedConnection & buf_con) = 0;
        virtual void onWriteDone(BufferedConnection & buf_con) = 0;
        virtual void onReadAvailable(BufferedConnection & buf_con) = 0;
        virtual void onError(BufferedConnection & buf_con, const std::string & what) = 0;
    };

}

#endif //HW_5_ISERVICELISTENER_H
