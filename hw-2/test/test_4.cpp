#include "log.h"

int main() {
    log::init_with_stdout_logger(log::LL_WARN);
    log::info("Hello ");
    log::debug("my baby, ");
    log::warn("hello ");
    log::error("my honey!");
    return 0;
}
