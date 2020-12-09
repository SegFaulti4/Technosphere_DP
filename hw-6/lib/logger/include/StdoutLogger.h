#ifndef HW_2_STDOUTLOGGER_H
#define HW_2_STDOUTLOGGER_H

#include "BaseLogger.h"

namespace log {

    class StdoutLogger : public BaseLogger {
    public:
        StdoutLogger() : BaseLogger(std::cout) {}

        explicit StdoutLogger(LoggerLevel lvl) : BaseLogger(std::cout, lvl) {}
    };

}

#endif //HW_2_STDOUTLOGGER_H
