#include "StderrLogger.h"

namespace log {

    StderrLogger::StderrLogger(Logger_level lvl) {
        level_ = lvl;
    }

    void StderrLogger::log(std::string &what, Logger_level lvl) {
        if (lvl >= level_) {
            std::cerr << LOGGER_LEVEL_OUT_TABLE[lvl] << what << std::endl;
        }
    }

    void StderrLogger::flush() {}

}
