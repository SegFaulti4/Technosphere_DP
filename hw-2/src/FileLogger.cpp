#include "FileLogger.h"

namespace log {

    void FileLogger::open(const std::string &path) {
        stream_.open(path);
    }

    void FileLogger::close() {
        stream_.close();
    }

    FileLogger::~FileLogger() {
        try {
            stream_.close();
        }
        catch (int) {}
    }

}
