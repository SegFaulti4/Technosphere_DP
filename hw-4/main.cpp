#include "shmem.h"
#include <unistd.h>

int main() {
    shmem::SharedMem mem(4, 2048);
    shmem::SharedMap<int, int> mymap(mem);
    if (fork() == 0) {
        for (int i = 0; i < 100; i++) {
            mymap.insert(i, i + 1);
        }
    } else {
        sleep(1);
        for (int i = 0; i < 100; i++) {
            auto val = mymap.get(i);
            std::cout << val->first << ":" << val->second << std::endl;
        }
    }
    return 0;
}
