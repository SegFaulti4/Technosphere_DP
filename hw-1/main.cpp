#include "Process.h"
#include <unistd.h>
#include <iostream>

int main() {
    Process cur("ls");
    char buf[2048];
    size_t res;
    while((res = cur.read(buf, 2048)) > 0) {
        std::cout << std::string(buf, res);
    }
    return 0;
}
