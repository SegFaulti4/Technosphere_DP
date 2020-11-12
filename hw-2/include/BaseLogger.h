#ifndef HW_2_BASELOGGER_H
#define HW_2_BASELOGGER_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>

namespace log {

    enum Logger_level {
        LL_ERROR = 0,
        LL_WARN = 1,
        LL_INFO = 2,
        LL_DEBUG = 3
    };

    class BaseLogger {
    private:
        std::ostream& stream_;
        Logger_level level_;

    protected:
        void log(const std::string & what, Logger_level lvl);
        std::vector<std::string> LOGGER_LEVEL_OUT_TABLE = {
                "error: ",
                "warn: ",
                "info: ",
                "debug: "
        };
        explicit BaseLogger(std::ostream & stream, Logger_level level = LL_INFO) : stream_(stream), level_(level) {}

    public:
        void debug(const std::string & what);
        void info(const std::string & what);
        void warn(const std::string & what);
        void error(const std::string & what);
        void set_level(Logger_level lvl);
        int level() const;
        void flush();
    };

}

#endif //HW_2_BASELOGGER_H
