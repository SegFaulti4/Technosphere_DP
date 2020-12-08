#ifndef LOG_LOGGER_H
#define LOG_LOGGER_H

#include "BaseLogger.h"
#include <memory>

namespace log {

    class Logger {
    public:
        static Logger & getInstance();
        BaseLogger & getGlobalLogger();
        void setGlobalLogger(std::unique_ptr<BaseLogger> bl);

        Logger(Logger const &) = delete;
        Logger(Logger &&) = delete;
        Logger &operator=(Logger const &) = delete;
        Logger &operator=(Logger &&) = delete;

    private:
        std::unique_ptr<BaseLogger> global_logger_ = nullptr;

        Logger() = default;
        ~Logger() = default;
    };

}

#endif //LOG_LOGGER_H
