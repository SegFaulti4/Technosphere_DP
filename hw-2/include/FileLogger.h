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

    public:
        explicit FileLogger(const std::string & path) : ofstream_(path), BaseLogger(ofstream_) {}

        explicit FileLogger(const std::string & path, Logger_level lvl) : ofstream_(path), BaseLogger(ofstream_, lvl) {}
    };

}

#endif //HW_2_FILELOGGER_H
