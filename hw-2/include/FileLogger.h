#ifndef HW_2_FILELOGGER_H
#define HW_2_FILELOGGER_H

#include "BaseLogger.h"
#include "Descriptor.h"
#include "fcntl.h"
#include "unistd.h"
#include <stdexcept>

namespace log {

    class FileLogger : public BaseLogger {
    private:
        Descriptor ofstream_;

        void log(std::string &what, Logger_level lvl) override;

    public:
        explicit FileLogger(const std::string &path, Logger_level lvl);

        ~FileLogger();

        void open(const std::string &path);

        void close();

        void flush() override;
    };

}

#endif //HW_2_FILELOGGER_H
