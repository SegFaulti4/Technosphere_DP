#include "Logger.h"

namespace log {

    Logger & Logger::getInstance() {
        static Logger log;
        return log;
    }

    BaseLogger& Logger::getGlobalLogger() {
        if (global_logger_ == nullptr) {
            throw std::runtime_error("Trying to return invalid logger\n");
        }
        return *global_logger_;
    }

    void Logger::setGlobalLogger(std::unique_ptr<BaseLogger> bl) {
        global_logger_ = std::move(bl);
    }

}
