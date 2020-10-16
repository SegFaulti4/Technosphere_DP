#include "FileLogger.h"
#include "fcntl.h"
#include <stdexcept>

namespace log {

    FileLogger::FileLogger(const std::string &path, Logger_level lvl) {
        ofstream_.open(path, O_WRONLY | O_APPEND | O_CREAT);
        level_ = lvl;
    }

    void FileLogger::flush() {
        ofstream_.flush();
    }

    void FileLogger::open(std::string &path) {
        ofstream_.open(path, O_WRONLY | O_APPEND | O_CREAT);
    }

    void FileLogger::close() {
        ofstream_.close();
    }

    FileLogger::~FileLogger() {
        FileLogger::flush();
        FileLogger::close();
    }

    void FileLogger::log(std::string &what, Logger_level lvl) {
        if (lvl >= level_) {
            ofstream_.write(LOGGER_LEVEL_OUT_TABLE[level_].data(),
                            LOGGER_LEVEL_OUT_TABLE[level_].length());
            ofstream_.write(what.data(), what.length());
        }
    }

}
