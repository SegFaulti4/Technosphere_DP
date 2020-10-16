#ifndef HW_2_FILELOGGER_H
#define HW_2_FILELOGGER_H

#include "BaseLogger.h"
#include "FileDes.h"

namespace log {

    class FileLogger : public BaseLogger {
    private:
        FileDes ofstream_;

        void log(std::string &what, Logger_level lvl) override;

    public:
        explicit FileLogger(const std::string &path, Logger_level lvl);

        ~FileLogger();

        void open(std::string &path);

        void close();

        void flush() override;
    };

}

#endif //HW_2_FILELOGGER_H
