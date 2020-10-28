#ifndef HW_2_STDOUTLOGGER_H
#define HW_2_STDOUTLOGGER_H

#include "BaseLogger.h"
#include <iostream>

namespace log {

    class StdoutLogger : public BaseLogger {
    private:
        void log(std::string &what, Logger_level lvl) override;

    public:
        StdoutLogger();
        explicit StdoutLogger(Logger_level lvl);

        void flush() override;
    };

}

#endif //HW_2_STDOUTLOGGER_H
