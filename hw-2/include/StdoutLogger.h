#ifndef HW_2_STDOUTLOGGER_H
#define HW_2_STDOUTLOGGER_H

#include "BaseLogger.h"
#include <iostream>

namespace log {

    class StdoutLogger : public BaseLogger {
    private:
        void log(std::string &what, Logger_level lvl) override;

    public:
        explicit StdoutLogger(Logger_level lvl = LL_DEBUG);

        void flush() override;
    };

}

#endif //HW_2_STDOUTLOGGER_H
