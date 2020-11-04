#include "Process.h"
#include <iostream>

int main() {
    Process cur("./bypass");
    std::string str = "Hello World!";
    cur.write(str.data(), str.size());
    char buf[2048] = {};
    cur.read(buf, 2047);
    std::cout << std::string(buf) << std::endl;
    return 0;
}
