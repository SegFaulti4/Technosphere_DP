#include "Logger.h"

namespace log {

    Logger & Logger::get_instance() {
        static Logger log;
        return log;
    }

    BaseLogger& Logger::get_global_logger() {
        if (global_logger_ == nullptr) {
            throw std::runtime_error("Trying to return invalid logger\n");
        }
        return *global_logger_;
    }

    void Logger::set_global_logger(std::unique_ptr<BaseLogger> bl) {
        global_logger_ = std::move(bl);
    }

}
