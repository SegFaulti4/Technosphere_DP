#ifndef HW_2_FILELOGGER_H
#define HW_2_FILELOGGER_H

#include "BaseLogger.h"
#include "fcntl.h"
#include "unistd.h"
#include <stdexcept>
#include <fstream>

namespace log {

    class FileLogger : public BaseLogger {
    private:
        std::ofstream ofstream_;
        std::ofstream& stream_;

    public:
        explicit FileLogger(const std::string & path) : ofstream_(path), stream_(ofstream_), BaseLogger(ofstream_) {}

        explicit FileLogger(const std::string & path, Logger_level lvl) : ofstream_(path), stream_(ofstream_), BaseLogger(ofstream_) {
            level_ = lvl;
        }

        ~FileLogger();

        void open(const std::string &path);
        void close();
    };

}

#endif //HW_2_FILELOGGER_H
