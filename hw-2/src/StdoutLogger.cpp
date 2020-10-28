#include "StdoutLogger.h"

namespace log {

    StdoutLogger::StdoutLogger() = default;

    StdoutLogger::StdoutLogger(Logger_level lvl) {
        level_ = lvl;
    }

    void StdoutLogger::log(std::string &what, Logger_level lvl) {
        if (level_ <= lvl) {
            std::cout << LOGGER_LEVEL_OUT_TABLE[lvl] << what << std::endl;
        }
    }

    void StdoutLogger::flush() {
        std::cout.flush();
    }
}
