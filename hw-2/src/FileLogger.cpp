#include "FileLogger.h"

namespace log {

    void FileLogger::open(const std::string &path) {
        ofstream_.open(path);
    }

    void FileLogger::close() {
        ofstream_.close();
    }

}
