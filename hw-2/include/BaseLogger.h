#ifndef HW_2_BASELOGGER_H
#define HW_2_BASELOGGER_H

#include <string>
#include <vector>

namespace log {

    enum Logger_level {
        LL_DEBUG = 0,
        LL_INFO = 1,
        LL_WARN = 2,
        LL_ERROR = 3
    };

    class BaseLogger {
    protected:
        Logger_level level_ = LL_DEBUG;

        virtual void log(std::string &what, Logger_level lvl) {};

        std::vector<std::string> LOGGER_LEVEL_OUT_TABLE = {
                "debug:",
                "info:",
                "warn:",
                "error:"
        };

    public:
        void debug(std::string &what);

        void info(std::string &what);

        void warn(std::string &what);

        void error(std::string &what);

        void set_level(Logger_level lvl);

        int level();

        virtual void flush() {};
    };

}

#endif //HW_2_BASELOGGER_H
