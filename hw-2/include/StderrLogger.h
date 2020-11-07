#ifndef HW_2_STDERRLOGGER_H
#define HW_2_STDERRLOGGER_H

#include "BaseLogger.h"
#include <iostream>

namespace log {

    class StderrLogger : public BaseLogger {
    public:
        StderrLogger() : BaseLogger(std::cerr) {}

        explicit StderrLogger(Logger_level lvl) : BaseLogger(std::cerr) {
            level_ = lvl;
        }
    };

}

#endif //HW_2_STDERRLOGGER_H
