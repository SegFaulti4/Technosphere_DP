#include "Global.h"

namespace log {

    void initWithStderrLogger(LoggerLevel lvl) {
        Logger::getInstance().setGlobalLogger(std::make_unique<StderrLogger>(lvl));
    }

    void initWithStdoutLogger(LoggerLevel lvl) {
        Logger::getInstance().setGlobalLogger(std::make_unique<StdoutLogger>(lvl));
    }

    void initWithFileLogger(const std::string & path, LoggerLevel lvl) {
        Logger::getInstance().setGlobalLogger(std::make_unique<FileLogger>(path, lvl));
    }

    void info(const std::string & what) {
        Logger::getInstance().getGlobalLogger().info(what);
    }

    void debug(const std::string & what) {
        Logger::getInstance().getGlobalLogger().debug(what);
    }

    void warn(const std::string & what) {
        Logger::getInstance().getGlobalLogger().warn(what);
    }

    void error(const std::string & what) {
        Logger::getInstance().getGlobalLogger().error(what);
    }

}
