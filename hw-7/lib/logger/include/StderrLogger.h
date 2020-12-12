#ifndef HW_2_STDERRLOGGER_H
#define HW_2_STDERRLOGGER_H

#include "BaseLogger.h"

namespace log {

    class StderrLogger : public BaseLogger {
    public:
        StderrLogger() : BaseLogger(std::cerr) {}

        explicit StderrLogger(LoggerLevel lvl) : BaseLogger(std::cerr, lvl) {}
    };

}

#endif //HW_2_STDERRLOGGER_H
