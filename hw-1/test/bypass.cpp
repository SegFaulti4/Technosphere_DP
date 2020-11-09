#include <unistd.h>

int main() {
    char buf[2048];
    size_t res;
    while((res = ::read(STDIN_FILENO,buf, 2048)) > 0) {
        ::write(STDOUT_FILENO, buf, res);
    }
    return 0;
}
