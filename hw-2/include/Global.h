#ifndef LOG_GLOBAL_H
#define LOG_GLOBAL_H

#include "Logger.h"
#include "BaseLogger.h"
#include "FileLogger.h"
#include "StderrLogger.h"
#include "StdoutLogger.h"

namespace log {

    void init_with_stderr_logger(Logger_level lvl = LL_INFO);
    void init_with_stdout_logger(Logger_level lvl = LL_INFO);
    void init_with_file_logger(const std::string & path, Logger_level lvl = LL_INFO);
    void info(const std::string & what);
    void debug(const std::string & what);
    void warn(const std::string & what);
    void error(const std::string & what);

}

#endif //LOG_GLOBAL_H
