#include "BaseLogger.h"

namespace log {

    void BaseLogger::debug(std::string &what) {
        log(what, LL_DEBUG);
    }

    void BaseLogger::info(std::string &what) {
        log(what, LL_INFO);
    }

    void BaseLogger::warn(std::string &what) {
        log(what, LL_WARN);
    }

    void BaseLogger::error(std::string &what) {
        log(what, LL_ERROR);
    }

    void BaseLogger::set_level(Logger_level lvl) {
        level_ = lvl;
    }

    int BaseLogger::level() {
        return level_;
    }

}
