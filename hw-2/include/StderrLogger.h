#ifndef HW_2_STDERRLOGGER_H
#define HW_2_STDERRLOGGER_H

#include "BaseLogger.h"
#include <iostream>

namespace log {

    class StderrLogger : public BaseLogger {
    private:
        void log(std::string &what, Logger_level lvl) override;

    public:
        StderrLogger();

        explicit StderrLogger(Logger_level lvl);

        void flush() override;
    };

}

#endif //HW_2_STDERRLOGGER_H
