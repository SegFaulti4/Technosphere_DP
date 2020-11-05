#include "Global.h"

namespace log {

    void init_with_stderr_logger(Logger_level lvl) {
        std::unique_ptr<StderrLogger> res (new StderrLogger(lvl));
        Logger & log = Logger::get_instance();
        log.set_global_logger(std::move(res));
    }

    void init_with_stdout_logger(Logger_level lvl) {
        std::unique_ptr<StdoutLogger> res (new StdoutLogger(lvl));
        Logger & log = Logger::get_instance();
        log.set_global_logger(std::move(res));
    }

    void init_with_file_logger(const std::string & path, Logger_level lvl) {
        std::unique_ptr<FileLogger> res (new FileLogger(path, lvl));
        Logger & log = Logger::get_instance();
        log.set_global_logger(std::move(res));
    }

    void info(const std::string & what) {
        Logger & log = Logger::get_instance();
        BaseLogger & baselogger = log.get_global_logger();
        baselogger.info(what);
    }

    void debug(const std::string & what) {
        Logger & log = Logger::get_instance();
        BaseLogger & baselogger = log.get_global_logger();
        baselogger.debug(what);
    }

    void warn(const std::string & what) {
        Logger & log = Logger::get_instance();
        BaseLogger & baselogger = log.get_global_logger();
        baselogger.warn(what);
    }

    void error(const std::string & what) {
        Logger & log = Logger::get_instance();
        BaseLogger & baselogger = log.get_global_logger();
        baselogger.error(what);
    }

}
