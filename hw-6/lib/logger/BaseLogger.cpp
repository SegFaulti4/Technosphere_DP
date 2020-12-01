#include "BaseLogger.h"

namespace log {

    void BaseLogger::debug(const std::string &what) {
        log(what, LL_DEBUG);
    }

    void BaseLogger::info(const std::string &what) {
        log(what, LL_INFO);
    }

    void BaseLogger::warn(const std::string &what) {
        log(what, LL_WARN);
    }

    void BaseLogger::error(const std::string &what) {
        log(what, LL_ERROR);
    }

    void BaseLogger::set_level(LoggerLevel lvl) {
        level_ = lvl;
    }

    int BaseLogger::level() const {
        return level_;
    }

    void BaseLogger::log(const std::string &what, LoggerLevel lvl) {
        if (level_ >= lvl) {
            stream_ << LOGGER_LEVEL_OUT_TABLE[lvl] << what << "\n";
        }
    }

    void BaseLogger::flush() {
        stream_.flush();
    }

}
