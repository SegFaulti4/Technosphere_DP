#ifndef LOG_LOGGER_H
#define LOG_LOGGER_H

#include "BaseLogger.h"

namespace log {

    class Logger {
    public:
        static Logger & get_instance();
        BaseLogger & get_global_logger();
        void set_global_logger(std::unique_ptr<BaseLogger> bl);

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
