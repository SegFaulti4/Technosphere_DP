#ifndef HW_2_STDOUTLOGGER_H
#define HW_2_STDOUTLOGGER_H

#include "BaseLogger.h"
#include <iostream>

namespace log {

    class StdoutLogger : public BaseLogger {
    public:
        StdoutLogger() : BaseLogger(std::cout) {}

        explicit StdoutLogger(Logger_level lvl) : BaseLogger(std::cout) {
            level_ = lvl;
        }
    };

}

#endif //HW_2_STDOUTLOGGER_H
