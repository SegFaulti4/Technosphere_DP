#include "log.h"
#include <string>

int main() {
    std::string str[] = {"I'm ", "blinded ", "by ", "the lights"};
    log::FileLogger SL1("out1.txt", log::LL_DEBUG);
    SL1.debug(str[0]);
    SL1.info(str[1]);
    SL1.warn(str[2]);
    SL1.error(str[3]);
    SL1.flush();

    log::FileLogger SL2("out2.txt", log::LL_INFO);
    SL2.debug(str[0]);
    SL2.info(str[1]);
    SL2.warn(str[2]);
    SL2.error(str[3]);
    SL2.flush();

    log::FileLogger SL3("out3.txt", log::LL_WARN);
    SL3.debug(str[0]);
    SL3.info(str[1]);
    SL3.warn(str[2]);
    SL3.error(str[3]);
    SL3.flush();

    log::FileLogger SL4("out4.txt", log::LL_ERROR);
    SL4.debug(str[0]);
    SL4.info(str[1]);
    SL4.warn(str[2]);
    SL4.error(str[3]);
    SL4.flush();
    return 0;
}
