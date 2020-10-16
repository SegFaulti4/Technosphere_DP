#include <iostream>
#include "./include/Process.h"
#include <unistd.h>

int main() {
    Process cur("ls");
    char buf[2048];
    size_t res;
    while((res = cur.read(buf, 2048)) > 0) {
        ::write(STDOUT_FILENO, buf, res);
    }
    return 0;
}
