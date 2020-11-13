#include "log.h"
#include <string>

int main() {
    log::StdoutLogger tmplSL(log::LL_WARN);
    std::string str = "Hello";
    tmplSL.warn(str);
    tmplSL.info(str);
    return 0;
}