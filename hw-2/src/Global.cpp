#include "Global.h"

namespace log {

    void init_with_stderr_logger(Logger_level lvl) {
        Logger::get_instance().set_global_logger(std::make_unique<StderrLogger>(lvl));
    }

    void init_with_stdout_logger(Logger_level lvl) {
        Logger::get_instance().set_global_logger(std::make_unique<StdoutLogger>(lvl));
    }

    void init_with_file_logger(const std::string & path, Logger_level lvl) {
        Logger::get_instance().set_global_logger(std::make_unique<FileLogger>(path, lvl));
    }

    void info(const std::string & what) {
        Logger::get_instance().get_global_logger().info(what);
    }

    void debug(const std::string & what) {
        Logger::get_instance().get_global_logger().debug(what);
    }

    void warn(const std::string & what) {
        Logger::get_instance().get_global_logger().warn(what);
    }

    void error(const std::string & what) {
        Logger::get_instance().get_global_logger().error(what);
    }

}
