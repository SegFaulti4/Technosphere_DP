#ifndef LOG_GLOBAL_H
#define LOG_GLOBAL_H

#include "Logger.h"
#include "BaseLogger.h"
#include "FileLogger.h"
#include "StderrLogger.h"
#include "StdoutLogger.h"

namespace log {

    void initWithStderrLogger(LoggerLevel lvl = LL_INFO);
    void initWithStdoutLogger(LoggerLevel lvl = LL_INFO);
    void initWithFileLogger(const std::string & path, LoggerLevel lvl = LL_INFO);
    void info(const std::string & what);
    void debug(const std::string & what);
    void warn(const std::string & what);
    void error(const std::string & what);

}

#endif //LOG_GLOBAL_H
