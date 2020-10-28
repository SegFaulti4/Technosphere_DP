#include "FileLogger.h"

namespace log {

    FileLogger::FileLogger(const std::string &path, Logger_level lvl) {
        open(path);
        level_ = lvl;
    }

    void FileLogger::flush() {
        if (::fsync(ofstream_.get_fd()) == -1) {
            throw std::runtime_error("Fsync error");
        }
    }

    void FileLogger::open(const std::string &path) {
        ofstream_.set_fd(::open(path.data(), O_WRONLY | O_APPEND | O_CREAT));
        if (ofstream_.get_fd() == -1) {
            throw std::runtime_error("Open error\n");
        }
    }

    void FileLogger::close() {
        ofstream_.close();
    }

    FileLogger::~FileLogger() {
        ::close(ofstream_.get_fd());
    }

    void FileLogger::log(std::string &what, Logger_level lvl) {
        if (level_ <= lvl) {
            ofstream_.write(LOGGER_LEVEL_OUT_TABLE[level_].data(),
                            LOGGER_LEVEL_OUT_TABLE[level_].length());
            ofstream_.write(what.data(), what.length());
            char tmp = '\n';
            ofstream_.write(&tmp, sizeof(tmp));
        }
    }

}
